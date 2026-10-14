/******************************************************************
 *                          Vampire.C                             *
 * Yeah man this has lots of vampire shit in yeah		  *
 *   ... ok and lots of other races but who cares		  *
 *  Copyright 1997 Adam Powell Productions.			  *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "magic.h"
#include "lookup.h"
#include "tables.h"

DECLARE_DO_FUN(do_propose	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_think		);
DECLARE_DO_FUN(do_fang		);

void do_deathpool(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if (ch->altitude != 0)
    {
	stc("Get down on the floor and do it!\n\r", ch);
	return;
    }

    if (is_item(ch))
    {
	stc("You are already in item form, and cannot do this.\n\r", ch);
	return;
    }

    obj = create_object(get_obj_index(107), 0);
    obj_to_room(obj, ch->in_room, ch->altitude);
    stc("You turn into a rainbow pool and splash to the floor.\n\r", ch);
    act("$n changes into a blob of colourful liquid, which splashes to the ground.", ch, NULL, NULL, TO_ROOM);
    char_to_obj(ch, obj);
    SET_BIT(obj->extra_flags3, ITEM_DESTROY_CHAR_RETURN);
}

void do_inhabit(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if ( !IS_INSUBSTANTIAL(ch) )
    {
	stc("You must be in mist form to inhabit an item.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Which item do you wish to inhabit?\n\r", ch);
	return;
    }

    if ( ( obj = get_obj_here(ch, argument) ) == NULL)
    {
	stc("That item is not here.\n\r", ch);
	return;
    }

    act("$n flows into $p.", ch, obj, NULL, TO_ROOM);
    act("You flow into $p.", ch, obj, NULL, TO_CHAR);
    char_to_obj(ch, obj);
}

void do_caust(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;

    if (argument[0] == '\0')
    {
	stc("What weapon do you wish to cover with venom?\n\r", ch);
	return;
    }

    if ( ( obj = get_obj_carry(ch, argument, ch) ) == NULL)
    {
	stc("You are not carrying that weapon.\n\r", ch);
	return;
    }

    if (obj->item_type != ITEM_WEAPON)
    {
	stc("That item is not a weapon.\n\r", ch);
	return;
    }

    if (IS_SET(obj->value[4], WEAPON_CAUSTIC))
    {
	stc("This weapon has already been covered with venom.\n\r", ch);
	return;
    }

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = ch->level;
    af.duration     = -1;
    af.location     = APPLY_HITROLL;
    af.modifier     = ch->power[DISC_DAEM_CORR] + 1;
    af.bitvector    = 0;
    affect_to_obj(obj,&af);

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = ch->level;
    af.duration     = -1;
    af.location     = APPLY_DAMROLL;
    af.modifier     = ch->power[DISC_DAEM_CORR] + 1;
    af.bitvector    = 0;
    affect_to_obj(obj,&af);

    SET_BIT(obj->value[4], WEAPON_CAUSTIC);
    SET_BIT(obj->value[4], WEAPON_POISON);

    act("You run your tongue over $p, and coat it with sickly venom.",
	ch, obj, NULL, TO_CHAR);
    act("$n runs $s tongue over $p, and coats it with sickly venom.",
	ch, obj, NULL, TO_ROOM);
    WAIT_STATE(ch, 8);
}

void do_shadowdemon(CHAR_DATA *ch, char *argument)
{
    int newrank = 0;

    if (ch->secondaryrace != RACE_DAEMON)
    {
	stc("Only demons can use this ability.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Syntax : shadowdemon <rank>\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "lemure")) newrank = 1;
    else if (!str_prefix(argument, "nupperibo")) newrank = 2;
    else if (!str_prefix(argument, "spinagon")) newrank = 3;
    else if (!str_prefix(argument, "barbazu")) newrank = 4;
    else if (!str_prefix(argument, "abishai")) newrank = 5;
    else if (!str_prefix(argument, "osyluth")) newrank = 6;
    else if (!str_prefix(argument, "hamatula")) newrank = 7;
    else if (!str_prefix(argument, "erinyes")) newrank = 8;
    else if (!str_prefix(argument, "amnizu")) newrank = 9;
    else if (!str_prefix(argument, "cornugon")) newrank = 10;
    else if (!str_prefix(argument, "gelugon")) newrank = 11;
    else if (!str_prefix(argument, "pit fiend")) newrank = 12;
    else
    {
	do_shadowdemon(ch, "");
	return;
    }

    if (newrank > ch->generation)
    {
	stc("You cannot morph into a higher ranking demon.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);
    stc("You slowly change into your new form.\n\r", ch);
    act("$n drops to the floor, dead.", ch, NULL, NULL, TO_ROOM);
    set_form(ch, FRM_LEMURE - 1 + newrank);
    act("You have become $n.", ch, NULL, NULL, TO_CHAR);
    act("$n rips its way out of the corpse on the floor.", ch, NULL, NULL, TO_ROOM);
}

void do_wither(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    bool saved = FALSE;
    AFFECT_DATA af;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to inflict the withering disease on?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	stc("You would not want this to happen to yourself.\n\r", ch);
	return;
    }

    if (IS_FLAG5(victim, AFF5_WITHERING))
    {
	stc("They are already affected by wither.\n\r", ch);
	return;
    }

    if (is_undead(victim))
    {
	stc("The undead are immune to withering.\n\r", ch);
	return;
    }

    saved = saves_spell(ch->level, victim, DAM_DISEASE);

    if (victim->power[DISC_ELAD_MELO] >= ch->power[DISC_DAEM_CORR])
	saved = TRUE;

    if (saved)
    {
        victim = ch;
	stc("The power has backfired onto you.\n\r", ch);
    }

    af.where     = TO_FLAG5;
    af.type      = 0;
    af.level     = ch->level;
    af.duration  = ch->power[DISC_DAEM_CORR];
    af.location  = APPLY_CON;
    af.modifier  = 0 - ch->power[DISC_DAEM_CORR];
    af.bitvector = AFF5_WITHERING;
    affect_join(victim, &af);

    af.where     = TO_FLAG5;
    af.type      = 0;
    af.level     = ch->level;
    af.duration  = ch->power[DISC_DAEM_CORR];
    af.location  = APPLY_DEX;
    af.modifier  = 0 - ch->power[DISC_DAEM_CORR];
    af.bitvector = AFF5_WITHERING;
    affect_join(victim, &af);

    af.where     = TO_FLAG5;
    af.type      = 0;
    af.level     = ch->level;
    af.duration  = ch->power[DISC_DAEM_CORR];
    af.location  = APPLY_STR;
    af.modifier  = 0 - ch->power[DISC_DAEM_CORR];
    af.bitvector = AFF5_WITHERING;
    af.name      = "withering";
    af.end_message = "You feel less blighted.";
    affect_name_join(victim, &af);

    act("$n's eyes glow red.", ch, NULL, NULL, TO_ROOM);

    stc("You are struck with a hideous withering disease.\n\r", victim);
    act("$n is struck by a hideous withering disease.", victim, NULL, NULL, TO_ROOM);

    if (ch != victim) set_fighting(ch, victim);
}

const char * realm[] =
{
    "flame", "water", "air", "earth", "entropy", "life"
};

void do_arcanum(CHAR_DATA *ch, char *argument)
{
    int toplevel, i, g;

    if (argument[0] == '\0' || !is_number(argument) )
    {
	stc("Syntax : arcanum <spell level>\n\r", ch);
	return;
    }

    if (ch->primary == 0)
    {
	stc("You must have a primary element to use this power.\n\r", ch);
	return;
    }

    switch (ch->primary)
    {
	case REALM_FLAME : toplevel = ch->fire; break;
	case REALM_AIR : toplevel = ch->air; break; 
	case REALM_WATER : toplevel = ch->water; break;
	case REALM_EARTH : toplevel = ch->earth; break;
	case REALM_ENTROPY : toplevel = ch->entropy; break;
	case REALM_LIFE : toplevel = ch->lifemagic; break;
  	default : toplevel = -1; break;
    }
 
    i = atoi(argument);

    if (i < 0)
    {
	do_arcanum(ch, "");
	return;
    }

    if (i > toplevel)
    {
	stc("You cannot invoke the arcanum for such high level spells.\n\r", ch);
	return;
    }

    stcf(ch, "The arcanum list for level %d reads : \n\n\r", i);

    for (g = 0 ; g < MAX_SKILL ; g++)
    {
	if (skill_table[g].spell_fun == NULL) continue;
	if (skill_table[g].name[0] == '\0') continue;

	if (skill_table[g].level == i)
	    stcf(ch, "   Spell '%s' located in realm '%s', %d%s circle.\n\r",
		skill_table[g].name, realm[skill_table[g].realm-1], i,
		(i == 1) ? "st" : (i == 2) ? "nd" : (i == 3) ? "rd" : "th" );
    }

}

/* Artificia level 10, ho ho ho, its good for the elves!!!! */
void do_infuse(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_STRING_LENGTH];
    sh_int type = 0;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0' || argument[0] == '\0')
    {
	stc("Syntax : infuse {haste windtomb fly armor invisibility} <item>\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "haste")) type = 1;
    else if (!str_prefix(arg1, "windtomb")) type = 2;
    else if (!str_prefix(arg1, "armor")) type = 3;
    else if (!str_prefix(arg1, "fly")) type = 5;
    else if (!str_prefix(arg1, "invisibility")) type = 4;
    else
    {
	do_infuse(ch, "");
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch) ) == NULL)
    {
	stc("You are not carrying that item.\n\r", ch);
	return;
    }
 
    if (obj->item_type != ITEM_ARMOR)
    {
	stc("You may only infuse pieces of armour.\n\r", ch);
	return;
    }

    /* Cant do it on nasty items */
    if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
    {
        act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
        act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room, ch->altitude );
        return;   
    }

    if (obj->perm_spell != -1)
    {
	stc("That item already has a spell imbued within.\n\r", ch);
	return;
    }

    if (ch->faith < 20)
    {
	stc("You need twenty faith points to imbue an item.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);
    ch->faith -= 20;
    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
    SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD); /* saNiTy? */

    if (type == 1)
    {
	act("You infuse a haste cantrip into $p.", ch, obj, NULL, TO_CHAR);
	act("$n infuse a haste cantrip into $p.", ch, obj, NULL, TO_ROOM);
        obj->perm_spell = ITEMA_HASTE; 
	return;
    }

    if (type == 2)
    {
	act("You contain the power of the hurricane within $p.", ch, obj, NULL, TO_CHAR);
	act("$n contains the power of the hurricane within $p.", ch, obj, NULL, TO_ROOM);
        obj->perm_spell = ITEMA_WINDTOMB; 
	return;
    }

    if (type == 3)
    {
	act("You magically augment the protection provided by $p.", ch, obj, NULL, TO_CHAR);
	act("$n magically augments the protection provided by $p.", ch, obj, NULL, TO_ROOM);
        obj->perm_spell = ITEMA_ARMOR; 
	return;
    }

    if (type == 4)
    {
	act("You infuse the power of invisibility into $p.", ch, obj, NULL, TO_CHAR);
	act("$n infuses $s faith into $p.", ch, obj, NULL, TO_ROOM);
        obj->perm_spell = ITEMA_INVISIBILITY; 
	return;
    }

    act("You lay your hands upon $p, it begins to hover in the air.", ch, obj, NULL, TO_CHAR);
    act("$n lays $s hands upon $p, which hovers in the air.", ch, obj, NULL, TO_ROOM);
    obj->perm_spell = ITEMA_FLY; 
    return;
}

/* Artificia level 4, bless or protection from evil */
void do_sanctify(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_STRING_LENGTH];
    sh_int type = 0;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0' || argument[0] == '\0')
    {
	stc("Syntax : sanctify {bless protection} <item>\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "bless")) type = 1;
    else if (!str_prefix(arg1, "protection")) type = 2;
    else
    {
	stc("You may only choose bless or protection.\n\r", ch);
	return;
    }

    if ( (obj = get_obj_carry(ch, argument, ch) ) == NULL)
    {
	stc("You are not carrying that item.\n\r", ch);
	return;
    }
 
    if (obj->item_type != ITEM_ARMOR
	&& obj->item_type != ITEM_WEAPON)
    {
	stc("You may only sanctify weapons and pieces of armour.\n\r", ch);
	return;
    }

    /* Cant do it on nasty items */
    if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
    {
        act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
        act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room, ch->altitude );
        return;   
    }

    if (obj->perm_spell != -1)
    {
	stc("That item already has a spell imbued within.\n\r", ch);
	return;
    }

    if (ch->faith < 10)
    {
	stc("You need ten faith points to sanctify an item.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);
    ch->faith -= 10;
    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
    SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
    REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD); /* saNiTy? */

    if (type == 1)
    {
	act("You imbue a blessing into $p.", ch, obj, NULL, TO_CHAR);
	act("$n imbues a blessing into $p.", ch, obj, NULL, TO_ROOM);
        obj->perm_spell = ITEMA_BLESS; 
	return;
    }

    act("You imbue a warding into $p.", ch, obj, NULL, TO_CHAR);
    act("$n imbues a warding into $p.", ch, obj, NULL, TO_ROOM);
    obj->perm_spell = ITEMA_PROTECTION_EVIL; 
    return;
}

void do_reform(CHAR_DATA *ch, char *argument)
{
    int dir;
    ROOM_INDEX_DATA *room;

    if (argument[0] == '\0')
    {
	stc("Which direction do you wish to reform in?\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_AIR || ch->altitude != 0)
    {
	stc("There is no ground to meld with.\n\r", ch);
	return;
    }

    if (IS_IN_WATER(ch))
    {
	stc("You cannot meld while in the water.\n\r", ch);
	return;
    }

    if (ch->riding != NULL)
    {
	stc("Not while mounted.\n\r", ch);
	return;
    }

    if (ch->riden_by != NULL)
    {
	stc("Not while being ridden.\n\r", ch);
	return;
    }

    if ( (dir = get_direction(argument) ) == -1)
    {
	stc("That is not a valid direction.\n\r", ch);
	return;
    }

    if (ch->in_room->exit[dir] == NULL)
    {
	stc("There is no room in that direction.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    act("$n melds into the ground.", ch, NULL, NULL, TO_ROOM);
    stc("You meld into the ground.\n\r", ch);
    room = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, room->exit[dir]->u1.to_room, 0);
    act("$n rises slowly out of the ground.", ch, NULL, NULL, TO_ROOM);
    stc("You rise out of the ground in a new place.\n\r", ch);
}

/* Here are a few things for DrOw */
int get_drow_magic_resistance(CHAR_DATA *ch)
{
    int res = 0;

    if (ch == NULL) return 0;

    if (IS_FLAG5(ch, AFF5_FAMILIAR))
	return ch->power[DISC_WARL_CONT] * 10;

    if (ch->tertiaryrace != TERTIARY_DROW) return 0;
    if (!IS_FLAG4(ch, AFF_GEN_RESIST)) return 0;

    if (ch->clan == CLAN_DROW_OUTCAST) res = 10;
    if (ch->clan == CLAN_DROW_FIRST) res = 60;
    if (ch->clan == CLAN_DROW_SECOND) res = 50;
    if (ch->clan == CLAN_DROW_THIRD) res = 40;
    if (ch->clan == CLAN_DROW_FOURTH) res = 30;
    if (ch->clan == CLAN_DROW_FIFTH) res = 20;

    res += ch->generation;

    return res;
}

void do_evileye(CHAR_DATA *ch, char *argument)
{
    NO_NPC(ch);

    if (argument[0] == '\0')
    {
	stc("Syntax : evileye [clear fright", ch);
	if (ch->power[DISC_DAEM_DISC] >= 5) stc(" appear", ch);
	if (ch->power[DISC_DAEM_DISC] >= 6) stc(" imbalance", ch);
	if (ch->power[DISC_DAEM_DISC] >= 7) stc(" cowardice", ch);
	if (ch->power[DISC_DAEM_DISC] >= 8) stc(" slumber", ch);
	if (ch->power[DISC_DAEM_DISC] >= 9) stc(" clumsy", ch);
	if (ch->power[DISC_DAEM_DISC] >= 10) stc(" anger", ch);
  	stc("]\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "clear"))
    {
	stc("Evileye cleared.\n\r", ch);
	free_string(ch->pcdata->evileye);
	ch->pcdata->evileye = NULL;
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "fright"))
    {
	stc("Fright evileye added.\n\r", ch);
	WRITE_STR(ch->pcdata->evileye, "flee");
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "slumber") && ch->power[DISC_DAEM_DISC] >= 8)
    {
	stc("Slumber evileye added.\n\r", ch);
	WRITE_STR(ch->pcdata->evileye, "sleep");
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "appear") && ch->power[DISC_DAEM_DISC] >= 5)
    {
	stc("Appear evileye added.\n\r", ch);
	WRITE_STR(ch->pcdata->evileye, "visible");
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "imbalance") && ch->power[DISC_DAEM_DISC] >= 6)
    {
	stc("Imbalance evileye added.\n\r", ch);
	WRITE_STR(ch->pcdata->evileye, "stance");
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "cowardice") && ch->power[DISC_DAEM_DISC] >= 7)
    {
	stc("Cowardice evileye added.\n\r", ch);
	WRITE_STR(ch->pcdata->evileye, "beg safety");
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "clumsy") && ch->power[DISC_DAEM_DISC] >= 9)
    {
	stc("Clumsy evileye added.\n\r", ch);
	WRITE_STR(ch->pcdata->evileye, "drop all");
	WAIT_STATE(ch, 8);
	return;
    }

    if (!str_prefix(argument, "anger") && ch->power[DISC_DAEM_DISC] >= 10)
    {
	char buf[100];

	stc("Anger evileye added.\n\r", ch);
	sprintf(buf, "kill %s", GET_NAME(ch));
	WRITE_STR(ch->pcdata->evileye, buf);
	WAIT_STATE(ch, 8);
	return;
    }

    do_evileye(ch, "");
    return;
}

void do_assail(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *gch, *victim;
    bool found = FALSE;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to assail?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    for (gch = char_list ; gch != NULL ; gch = gch->next)
    {
	if (is_same_group(ch, gch)
	    && ch != gch
	    && ch->altitude == gch->altitude
	    && ch->in_room == gch->in_room)
	{
	    found = TRUE;
	    act("$n suddenly lunges upon $N.", gch, NULL, victim, TO_NOTVICT);
	    act("$n suddenly lunges upon you.", gch, NULL, victim, TO_VICT);
	    act("You suddenly feel the urge to attack $N.", gch, NULL, victim, TO_CHAR);
	    set_fighting(gch, victim);
	}
    }

    if (!found)
	stc("There are no followers present.\n\r", ch);
}



void do_web(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to fire a web at?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	stc("You cannot fire webs at yourself!\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag5, AFF5_WEB_WEAK)
	|| IS_FLAG5(victim, AFF5_WEB_STRONG) )
    {
	stc("Your victim is already webbed.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if (dice(1, 3) == 1)
    {
	stc("Your web shooter has blocked.\n\r", ch);
	return;
    }

    act("$n fires a sticky web at $N.", ch, NULL, victim, TO_NOTVICT);
    act("You fire a sticky web at $N.", ch, NULL, victim, TO_CHAR);
    act("$n fires a sticky web at you.", ch, NULL, victim, TO_VICT);

    if (IS_INSUBSTANTIAL(victim))
    {
	act("The web flies straight throught $n", victim, NULL, NULL, TO_ROOM);
	stc("The web flies straight through you.\n\r", victim);
	return;
    }

    stc("You are coated in a sticky morass of webs.\n\r", victim);
    act("$n is covered in a sticky morass of webs.", victim, NULL, NULL, TO_ROOM);

    af.where = TO_FLAG5;
    af.type = 0;
    af.level = ch->level;
    af.location = APPLY_AC;
    af.duration = (ch->generation / 2) + 1;
    af.bitvector = AFF5_WEB_WEAK;
    af.modifier = 80;
    af.name = "webs (weak)";
    af.end_message = "The webs surrounding you disintegrate.";
    affect_name_to_char(victim, &af);

    if (ch->power[DISC_DAEM_CORR] >= 7)
    {
        af.where = TO_FLAG5;
        af.type = 0;
        af.level = ch->level;
        af.location = APPLY_AC;
        af.duration = (ch->generation / 3);
        af.bitvector = AFF5_WEB_STRONG;
        af.modifier = 80;
        af.name = "webs (strong)";
        af.end_message = "The webs surrounding you disintegrate.";
        affect_name_to_char(victim, &af);
    }
}

void do_stench(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        stc("It wouldn't be polite to create a stench within a safe room.\r\n", ch);
        return;
    }

    stc("You exude toxic vapours from your skin.\n\r", ch);
    ADD_RTIMER(ch->in_room, RTIMER_STINKING_CLOUD, dice(1, 4) + 1);
    room_message(ch->in_room, "Stinking green vapours start to fill the room.");
} 

void do_clanhelp(CHAR_DATA *ch, char *argument)
{
    int clan;
    char buf[MAX_STRING_LENGTH];  

    if (argument[0] == '\0')
    {
	stc("What clan do you wish to find help on?\n\r", ch);
	return;
    }

    if (( clan = clan_lookup(argument)) == 0)
    {
	stc("No such clan exists.\n\r", ch);
	return;
    }

    if (clan_table[clan].disc1 == 0
        || clan_table[clan].disc2 == 0
        || clan_table[clan].disc3 == 0)
    {
	stc("That is not a proper clan.\n\r", ch);
	return;
    }

    stcf(ch, "This is the legacy of clan %s.\n\r", 
	clan_table[clan].name);

    if (!str_cmp(clan_data[clan].description, "<none>"))
	stc("No description has been entered yet.\n\r", ch);
    else stc(clan_data[clan].description, ch);
    stc("\n\r", ch);

    if (!str_cmp(clan_data[clan].leader, "<none>"))
	stc("This clan currently has no leader.\n\r", ch);
    else stcf(ch, "The leader of the clan is %s.\n\r", clan_data[clan].leader); 
    stc("\n\r", ch);


    stcf(ch, "The clan is currently allied with %s.\n\r",
	clan_ally(clan) );
    stcf(ch, "The clan is currently at war with %s.\n\r",
	clan_fighting(clan) );

    sprintf(buf, "Clan disciplines upon entering %s are as follows:\n\r",
	clan_table[clan].name);
    stc(buf, ch);
    sprintf(buf, "%s, %s and %s.\n\r",
	discipline[clan_table[clan].disc1],
	discipline[clan_table[clan].disc2],
	discipline[clan_table[clan].disc3]);
    stc(buf, ch);
}

void do_deathsense(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];
 
    WAIT_STATE(ch, 12);

    stc("You close your eyes and attempt to sense death.\n\r", ch);
    act("$n's eyes momentarily flicker.", ch, NULL, NULL, TO_ROOM);

    for (gch = char_list ; gch != NULL ; gch = gch->next )
    {
	if (gch != ch
	    && gch->in_room->area == ch->in_room->area
	    && can_see(ch, gch)
	    && is_undead(gch))
	{
	    sprintf(buf, "%s - %s.\n\r",
		GET_NAME(gch),
		gch->in_room->name);
	    stc(buf, ch);
	}
    }
}

void do_backfire(CHAR_DATA *ch, char *arguement)
{
    if (IS_FLAG5(ch, AFF5_BACKFIRE))
    {
	stc("Your backfire shield is already in operation.\n\r", ch);
	return;
    }

    if (ch->sapphires <= 0)
    {
	stc("You need a single sapphire to cast this spell.\n\r", ch);
	return;
    }

    ch->sapphires--;
    stc("You crush the sapphire into fine blue powder.\n\r", ch);
    stc("An invisible shield forms around you.\n\r", ch);
    act("$n crushes a sapphire into a fine blue powder.", ch, NULL, NULL, TO_ROOM);
    SET_BIT(ch->flag5, AFF5_BACKFIRE);
    WAIT_STATE(ch, 10);
}

void do_aroma(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to use this power on?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	stc("You smell nothing.\n\r", ch);
	return;
    }

    if (victim->power[DISC_WARL_THEA] >= 7 || victim->desc == NULL)
    {
	stc("You cannot make out any distinct aroma.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);

    victim->desc->as = ch->desc;
    interpret(victim, "learn");
    victim->desc->as = NULL;
}

/* Element level 5 - haw haw haw */
void do_reveal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    bool said = FALSE;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to use this power on?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	stc("You already know your own weaknesses.\n\r", ch);
	return;
    }

    if (check_immune(victim, DAM_FIRE) == IS_VULNERABLE)
    {
	do_think(victim, "I hope they don't realise that I am vulnerable to fire.");
	said = TRUE;
    }
    if (check_immune(victim, DAM_COLD) == IS_VULNERABLE)
    {
	do_think(victim, "I hope they don't realise that I am vulnerable to cold.");
	said = TRUE;
    }
    if (check_immune(victim, DAM_ACID) == IS_VULNERABLE)
    {
	do_think(victim, "I hope they don't realise that I am vulnerable to acid.");
	said = TRUE;
    }
    if (check_immune(victim, DAM_LIGHTNING) == IS_VULNERABLE)
    {
	do_think(victim, "I hope they don't realise that I am vulnerable to lightning.");
	said = TRUE;
    }

    if (!said)
	do_think(victim, "Im not really vulnerable to any elements.");

    WAIT_STATE(ch, 8);
}


/* Dominate level 7, and temptation level 5 */
void do_insight(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to use this power on?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	stc("You already know your own weaknesses.\n\r", ch);
	return;
    }

    if (victim->desc == NULL)
    {
	stc("You cannot fathom out any weaknesses.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);

    act("Your evil tendrils snake into $N's mind.",ch, NULL, victim, TO_CHAR);

    victim->desc->as = ch->desc;
    interpret(victim, "immune");
    victim->desc->as = NULL;
}

void do_divine(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to use this power on?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch == victim)
    {
	interpret(ch, "spells");
	return;
    }

    if (victim->power[DISC_WARL_THEA] >= 6 || victim->desc == NULL)
    {
	stc("You cannot divine what spells they have memorized.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);

    victim->desc->as = ch->desc;
    interpret(victim, "spells");
    victim->desc->as = NULL;
}

void do_darkness(CHAR_DATA *ch, char *argument)
{
    ADD_RTIMER(ch->in_room, RTIMER_DARK_ROOM, dice(1, 3) + 1);
    room_message(ch->in_room, "The room suddenly turns pitch black.");
    WAIT_STATE(ch, 12);
}

bool clear_some_spells(CHAR_DATA *ch, int percent)
{
    int i, j, k;
    bool changed = FALSE;

    if (IS_NPC(ch)) return FALSE;

    for ( i = 0 ; i < percent ; i++ )
    {
	j = dice(1, 11) - 1;
	k = dice(1, 11) - 1;

	if (ch->pcdata->memorize[j][k] != -1)
	{
	    stcf(ch, "You have lost the spell of %s.\n\r",
		skill_table[ch->pcdata->memorize[j][k]].name);
	    changed = TRUE;
	    ch->pcdata->memorize[j][k] = -1;
	}
    }
    return changed;
}

void do_forget(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    bool changed = FALSE;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make forget spells.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL )
    {
	stc("They aren't here.\n\r", ch);
	return;
    } 

    if (IS_NPC(victim))
    {
	stc("They haven't got any spells memorised.\n\r", ch);
	return;
    }

    /* 120 bit for testing purposes only */
    if (ch == victim && ch->level != 120)
    {
	stc("Try as you might, you cannot forget yourself.\n\r", ch);
	return;
    }

    changed = clear_some_spells(victim, ch->power[DISC_DAEM_CORR] * 3);

    WAIT_STATE(ch, 12);

    act("$n stares intently at $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n stares intently at you.", ch, NULL, victim, TO_VICT);
    act("You stare intently at $N.", ch, NULL, victim, TO_CHAR);
    if (changed)
    {
	stc("You suddenly feel a sharp stabbing pain in your head.\n\r", victim);
	act("You can hear $N's brain fizzle and short-circuit.", ch, NULL, victim, TO_CHAR);
    }
}

void do_bane(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to blight with bane.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL )
    {
	stc("They aren't here.\n\r", ch);
	return;
    } 

    if (IS_FLAG5(victim, AFF5_BANE) )
    {
	stc("They are already afflicted.\n\r", ch);
	return;
    }

    if (victim->secondaryrace != RACE_ELADRIN)
    {
	stc("It is only useful to use this power on eladrin.\n\r", ch);
	return;
    }

    if (victim->power[DISC_ELAD_AURA] == 0 || victim->aura == 0)
    {
	stc("They have no aura to affect.\n\r", ch);
	return;
    }

    if (victim->power[DISC_ELAD_MELO] >= 8)
    {
	do_say(victim, "Ellarabeth an helos na");
	stc("They seem unaffected.\n\r", ch);
	return;
    }

    act("Beams of red light shoot out of $n's eyes and hit $N's aura.", ch, NULL, victim, TO_NOTVICT);
    act("Beams of red light shoot out of your eyes and hit $N's aura.", ch, NULL, victim, TO_CHAR);
    act("Beams of red light shoot out of $n's eyes and hit your aura.", ch, NULL, victim, TO_VICT);
    room_message(ch->in_room, "The aura disintegrates.");
    victim->aura = 0;

    af.where = TO_FLAG5;
    af.type = 1;
    af.level = ch->level;
    af.location = APPLY_HITROLL;
    af.duration = (ch->generation / 2) + 1;
    af.bitvector = AFF5_BANE;
    af.modifier = -10;
    af.name = "bane";
    af.end_message = "You feel less blighted.";
    affect_name_join(victim, &af);
}

void do_discord(CHAR_DATA *ch, char *argument)
{
    if (!TIME_UP(ch, TIMER_DISCORD))
    {
	stc("You cannot use the discord power for another couple of ticks.\n\r", ch);
	return;
    }

    room_message(ch->in_room, "You hear loud banging noises and crashes all around you.");
    ADD_RTIMER(ch->in_room, RTIMER_DISCORD, dice(1, 3));
    WAIT_STATE(ch, 12);
    ADD_TIMER(ch, TIMER_DISCORD, 2);
}

void do_food(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int i;

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
	stc("Use this power on who?\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Syntax : food <character> <discipline>\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, arg1) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (victim->hit > -5)
    {
	stc("You can only use this on a mortally wounded player.\n\r", ch);
	return;
    }

    for ( i = 0 ; i < MAX_DISCIPLINES ; i++ )
    {
	if (discipline[i][0] != '\0'
	    && !str_prefix(argument, discipline[i])
	    && victim->power[i] > 0 )
	{
	    sprintf(buf, "You feel less knowledgeable about %s.\n\r", discipline[i]);
	    stc(buf, victim);
	    act("$n bends down and rips the knowledge out of $N's head.", ch, NULL, victim, TO_NOTVICT);
	    sprintf(buf, "You bend down and rip the %s out of %s's mind.\n\r", discipline[i], GET_NAME(victim));
	    stc(buf, ch);
	    victim->power[i] /= 2; /* Narsty */
	    if (victim->power[i] == 1) victim->power[i] = 0;
   	    ch->power[DISC_VAMP_SATY] = 9;
    	    WAIT_STATE(ch, 15);
	    return;
	}
    }
    stc("They do not know a discipline of that name.\n\r", ch);
}

void do_unnerve(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to unnerve?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 7);
    do_say(ch,"oooOOOoooOOOooo");
    set_stance(victim, 0);
}

void do_airblood(CHAR_DATA *ch, char *argument)
{
    if (IS_FLAG5(ch, AFF5_AIR_BLOOD))
    {
	stc("Your blood returns to normal.\n\r", ch);
	REMOVE_BIT(ch->flag5, AFF5_AIR_BLOOD);
	change_upkeep(ch, -7);
	WAIT_STATE(ch, 8);
	return;
    }

    change_upkeep(ch, 7);
    WAIT_STATE(ch, 8);
    SET_BIT(ch->flag5, AFF5_AIR_BLOOD);
    stc("You place air bubbles in your blood stream.\n\r", ch);
}

void do_maw(CHAR_DATA *ch, char *argument)
{
    if (IS_FLAG5(ch, AFF5_MAW))
    {
	stc("Your teeth retract and your mouth returns to normal.\n\r", ch);
	REMOVE_BIT(ch->flag5, AFF5_MAW);
	REMOVE_BIT(ch->flag2, AFF_BITE);
	WAIT_STATE(ch, 8);
	return;
    }

    if (!IS_NEWFLAG(ch, AFF_BITE))
        do_fang(ch, "");

    WAIT_STATE(ch, 8);
    SET_BIT(ch->flag5, AFF5_MAW);
    stc("You mouth enlarges into a gaping maw full of teeth.\n\r", ch);
}

void do_beast(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
	victim = ch;

    else if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (IS_FLAG5(victim, AFF5_BEAST) || victim->rage > 0)
    {
	if (victim != ch)
	{
	    stc("They are already in a frenzy.\n\r", ch);
	    return;
	}
	else
	{
	    stc("You are already in a frenzy.\n\r", ch);
	    return;
	}
    }

    if (victim != ch)
    {
        act("You whisper words to $N.", ch, NULL, victim, TO_CHAR);
        act("$n whispers words to $N.", ch, NULL, victim, TO_NOTVICT);
        act("$n whispers words to you.", ch, NULL, victim, TO_VICT);
	stc("You start snarling angrily.\n\r", victim);
    }
    else
    {
	stc("You start snarling angrily.\n\r", ch);
	act("$n starts snarling angrily.", ch, NULL, NULL, TO_ROOM);
    }

    victim->rage = UMAX(victim->beast, 10);
    SET_BIT(victim->flag5, AFF5_BEAST);
}



void do_song(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
	victim = ch;

    else if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    /* Heroes cannot do this on themselves until level 7 */
    if (victim == ch && ch->secondaryrace == RACE_HERO
	&& ch->power[DISC_HERO_VIVI] < 7)
    {
	stc("You lack the power to use this upon yourself.\n\r", ch);
	return;
    }

    if (!IS_FLAG5(victim, AFF5_BEAST))
    {
	if (victim != ch)
	{
	    stc("They are fully in control of themself.\n\r", ch);
	    return;
	}
	else
	{
	    stc("You are fully in control of yourself.\n\r", ch);
	    return;
	}
    }

    if (victim != ch)
    {
        act("You whisper soothing words to $N.", ch, NULL, victim, TO_CHAR);
        act("$n whispers soothing words to $N.", ch, NULL, victim, TO_NOTVICT);
        act("$n whispers soothing words to you.", ch, NULL, victim, TO_VICT);
	stc("You feel calmer.\n\r", victim);
    }
    else
    {
	stc("You calm yourself out of your rage.\n\r", ch);
	act("$n calms down out of $s rage.", ch, NULL, NULL, TO_ROOM);
    }

    defang(victim);
    declaw(victim);
    affect_strip(victim, gsn_fear);
    REMOVE_BIT(victim->flag2, AFF_FEAR);
    victim->rage = 0;
    REMOVE_BIT(victim->flag5, AFF5_BEAST);
}

void do_succulence(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make succulent?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (!IS_FLAG4(victim, AFF_BECKONED))
    {
	stc("You may only perform this on a beckoned animal.\n\r", ch);
	return;
    }

    if (IS_FLAG5(victim, AFF5_SUCCULENCE))
    {
	stc("Their blood is already succulent.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 8);
    victim->blood = 150;
    SET_BIT(victim->flag5, AFF5_SUCCULENCE);
    act("You subconciously increase the quality of $N's blood.", ch, NULL, victim, TO_CHAR);
    act("$n's eyes flicker momentarily.", ch, NULL, NULL, TO_ROOM);
}

void do_love(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH]; 

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make love you?\n\r", ch);
	return;
    }

    if ((victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
 	return;
    }

    if (victim->position < POS_STUNNED)
    {
	stc("They are in no fit state to love you.\n\r", ch);
	return;
    }
  
    if (victim->sex == ch->sex)
    {
	stc("Erm, maybe this would be a bad idea.\n\r", ch);
	return;
    }

    if (victim->secondaryrace == RACE_DAEMON
	&& victim->cur_form == FRM_ERINYES)
    {
	stc("You feel overwhelmed by the erinyes.\n\r", ch);
	sprintf(buf, "%s", GET_PROPER_NAME(victim));
	do_propose(ch, buf);
	return;
    }

    if (victim->position < POS_STANDING)
    {
	sprintf(buf, "%s", GET_PROPER_NAME(ch));
	do_stand(victim, "");
	return;
    }

    sprintf(buf, "%s", GET_PROPER_NAME(ch));
    do_propose(victim, buf);
}

void taint_untransform(CHAR_DATA *ch)
{
    int i;
 
    crash("taint untransform");

    if (TIMER(ch, TIMER_TAINT) == 0) return;

    ch->tertiaryrace = 0;
    ch->secondaryrace = 0;
    ch->clan = 0;
    ch->generation = 0;
    SET_TIMER(ch, TIMER_TAINT, 0);

    /* wipe disciplines */
    for (i = 0 ; i < MAX_DISCIPLINES ; i++)
        ch->power[i] = -2;

    set_form(ch, 0);
    ch->blood = 100;
    save_char_obj(ch);
}


void taint_transform(CHAR_DATA *ch, int race)
{
    crash("taint transform");

    if (TIMER(ch, TIMER_TAINT) > 0) return;
    if (ch->secondaryrace != 0) return;

    if (race == RACE_HERO || race == RACE_DRAGON) return;

    ch->tertiaryrace = 0;
    SET_TIMER(ch, TIMER_TAINT, 50);
    ch->secondaryrace = race;
    set_learnable_disciplines(ch, race);
    ch->clan = 0;
    ch->generation = 1;

    if (race == RACE_DAEMON)
 	set_form(ch, FRM_LEMURE);
    else if (race == RACE_ELADRIN)
 	set_form(ch, FRM_PIXIE);
    else set_form(ch, 0);

    ch->blood = 100;
    save_char_obj(ch);
}


int max_blood_pool(CHAR_DATA *ch)
{
    if (ch->tertiaryrace == TERTIARY_CRIMSON)
	return 190 + (ch->generation * (ch->generation - 1) * 5);

    if (ch->secondaryrace == RACE_VAMPIRE)
	return 100 + ((ch->generation - 1 ) * ch->generation * 5);

    return 40 + (get_size(ch) * 30);
}

/* Drain - levels 1, 3 and 4 of Anime and quite nasty */

void do_drain(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim = NULL;

    if (argument[0] == '\0')
    {
	if (ch->power[DISC_VAMP_ANIME] < 3)
	    stc("Who do you wish to drain of vitality?\n\r", ch);
	else stc("Syntax : drain [victim corpse]\n\r", ch);
	return;
    }

    if (ch->power[DISC_VAMP_ANIME] < 3)
    {
	if ((victim = get_char_room(ch, argument)) == NULL)
	{
	    stc("They aren't here.\n\r", ch);
	    return;
	}

        if (is_safe(ch, victim)) return;

	WAIT_STATE(ch, 10);

	act("$n stares intently at $N.", ch, NULL, victim, TO_NOTVICT);
	act("You stare intently at $N.", ch, NULL, victim, TO_CHAR);
	act("$n stares intently at you.", ch, NULL, victim, TO_VICT);

	if (victim->move > 50)
	{
	    victim->move -= dice(5, 10);
	    gain_move(ch, dice(5, 10));
	    stc("You feel refreshed.\n\r", ch);
	    stc("You start to feel weary.\n\r", victim);
	}
	else stc("Nothing seemed to happen.\n\r", ch);

	return;
    }

    /* put in the new thingys */

}




