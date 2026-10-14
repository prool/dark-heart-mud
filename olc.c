/******************************************************************
 * OLC  --  (C) 1996, by Chris Mellish		                  *
 *  and more importantly by Amdp Plowel				  *
 * Brought to you in LoOpY ViSiOn                                 *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "tables.h"
#include "lookup.h"

/* imported functions needed */
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_prefix	);
DECLARE_DO_FUN( do_newroom	);

extern AREA_DATA * area_last;
extern AREA_DATA * area_first;

void olc_save_mob_progs  args( ( void ) );
void olc_save_helps	 args( ( void ) );
void olc_save_shop	 args( ( FILE *fp, SHOP_DATA *shop ) );

/* List of creatable objects ....
   name, item_type, wear_flags, size, material, weight (10th pounds)
	cost (copper pieces), value 0, 1, 2, 3, 4 */

const struct stdobj_type stdobj_data[MAX_STDOBJ] =
{
    { "", 0, 0, 0, "", 0, 0, 0, 0, 0, 0, 0 },
    { "leather armor", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"leather", 75, 250, 4, 5, 4, 0, 2 },
    { "ringmail armor", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"metal", 150, 5000, 8, 8, 7, 0, 2 },
    { "padded armor", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"leather", 50, 200, 6, 6, 7, 1, 2 },
    { "plate armor", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"metal", 300, 30000, 16, 16, 17, 8, 3 },
    { "full plate armor", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"metal", 400, 500000, 20, 20, 20, 15, 4 },
    { "banded armor", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"metal", 200, 10000, 12, 13, 12, 4, 2 },
    { "brigandine", ITEM_ARMOR, ITEM_WEAR_BODY, SIZE_MEDIUM,
	"metal", 200, 5000, 8, 9, 7, 2, 2 }


};

void do_findfreernum(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *area;
    int vnum;
    bool found;

    stc("Free vnum ranges for areas are as follows:\n\r", ch);
    for (vnum = 50; vnum < 30000; vnum += 100)
    {
	found = FALSE;
	for (area = area_first; area != NULL; area = area->next)
	{
	    if (found) continue;
	    if (vnum < area->min_vnum || vnum > area->max_vnum )
	    {
		found = TRUE;
		stcf(ch, "    Vnum range : %d to %d\n\r", vnum - 50, vnum + 49);
	    }
	}
    }
}

void do_makebuilder(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (!IS_CREATOR(ch)) return;

    if (argument[0] == '\0')
    {
	stc("Who do you wish to make a builder.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_world(ch, argument) ) == NULL )
    {
	stc("They do not exist.\n\r", ch);
	return;
    }

    if (IS_NPC(victim) )
    {
	stc("Im not very sure that they would make a good builder.\n\r", ch);
	return;
    }

    if (CAN_BUILD(victim))
    {
	act("Building priviliges for $N revoked.", ch, NULL, victim, TO_CHAR);
	stc("You are no longer a builder.  Are you THAT bad?\n\r", victim);
	REMOVE_BIT(victim->flag6, AFF6_CAN_BUILD);
	return;
    }
    act("Building priviliges for $N added.", ch, NULL, victim, TO_CHAR);
    stc("You are now a builder.  Hurray.\n\r", victim);
    SET_BIT(victim->flag6, AFF6_CAN_BUILD);
}

OBJ_DATA * create_std_object (int type, int condition)
{
    OBJ_DATA *obj;

    obj = create_object( get_obj_index(98), 0);

    /* set all the items stuff */
    free_string(obj->name);
    obj->name = str_dup(stdobj_data[type].name);
    free_string(obj->short_descr);
    obj->short_descr = str_dup(stdobj_data[type].name);
    free_string(obj->description);
    obj->description = str_dup(stdobj_data[type].name);
    free_string(obj->material);
    obj->material = str_dup(stdobj_data[type].material);

    obj->wear_flags = stdobj_data[type].wear_flags;
    if (obj->wear_flags != 0)
	obj->wear_flags += ITEM_TAKE;

    obj->item_type = stdobj_data[type].item_type;
    obj->weight = stdobj_data[type].weight;
    obj->cost = stdobj_data[type].cost;

    obj->value[0] = stdobj_data[type].value1;
    obj->value[1] = stdobj_data[type].value2;
    obj->value[2] = stdobj_data[type].value3;
    obj->value[3] = stdobj_data[type].value4;
    obj->value[4] = stdobj_data[type].value5;
    return obj;
}

void do_oolc(CHAR_DATA *ch, char *argument)
{
    char buf1[MAX_INPUT_LENGTH];
/*    char buf2[MAX_INPUT_LENGTH];
    char *wholearg = argument; */
    OBJ_INDEX_DATA *obj = NULL;

    if (ch->in_room->area != ch->zone)
	ch->zone = ch->in_room->area;

    if (ch->in_room == NULL || ch->zone == NULL
	|| ch->in_room->area == NULL)
    {
	send_to_char("You are not currently in a zone or room.\n\r",ch);
	do_prefix(ch, "");
	return;
    }

    if (ch->zone->realm->vnum != ch->build_realm && ch->level != 120)
    {
	send_to_char("You do not have building rights in this area.\n\r",ch);
	do_prefix(ch, "");
	return;
    }

    if (argument[0] == '\0')
    {
	if (ch->prefix != NULL && !str_cmp(ch->prefix, "oolc "))
	{
	    send_to_char("#2Ending ObjectOLC editing mode.#n\n\r",ch);
	    do_prefix(ch, "");
	    REMOVE_BIT(ch->flag5, AFF5_BUILDING);
	    ch->pcdata->curr_vnum_oedit = -1;
	}
	else
        {
	    do_prefix(ch, "oolc ");
	    send_to_char("#2Welcome to ObjectOLC editing mode.#n\n\r",ch);
	    SET_BIT(ch->flag5, AFF5_BUILDING);
	}
	return;
    }

    if (ch->prefix != NULL && str_cmp(ch->prefix, "oolc "))
    {
	do_prefix(ch, "oolc ");
	send_to_char("#2Welcome to ObjectOLC editing mode#n.\n\r",ch);
    	SET_BIT(ch->flag5, AFF5_BUILDING);
    }
	
    if (!str_prefix(argument, "help") || argument[0] == '?')
    {
	stc("Syntax: oolc [create list]\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "north")
	|| !str_prefix(argument, "south")
	|| !str_prefix(argument, "east")
	|| !str_prefix(argument, "west")
	|| !str_prefix(argument, "look")
	|| !str_prefix(argument, "up")
	|| !str_prefix(argument, "down"))
    {
	interpret(ch, argument);
	return;
    }

    argument = one_argument(argument, buf1);

    if (!str_prefix(buf1, "create"))
    {
	if (argument[0] == 0)
	{
	    stc("Syntax:  create <new object name>\n\r", ch);
	    return;
	}

	if (olc_create_obj( ch, argument ) == NULL)
	{
	    stc("Creation operation failed.\n\r", ch);
	    return;
	}
	stc("Object sucessfully created.\n\r", ch);
	stcf(ch, "Current editing Object vnum set to %d.\n\r",
	    ch->pcdata->curr_vnum_oedit);
	olc_save_zone(ch->zone, ch);
	return;
    }

    if (!str_prefix(buf1, "list"))
    {
	int vnum, count = 0;

	for (vnum = ch->zone->min_vnum; vnum <= ch->zone->max_vnum; vnum++)
	{
	    obj = get_obj_index( vnum );
	    if (obj == NULL || str_prefix(argument, obj->name))
		continue;

	    stcf(ch, "Vnum %5d : %s\n\r", vnum, obj->name);
	    count++;
	}
	if (count == 0)
	    stc("No objects were found.\n\r", ch);
	else
	    stcf(ch, "%d object matching pattern '%s*' found.\n\r", count, argument);
	return;
    }

    if (!str_prefix(buf1, "select"))
    {
  	int vnum;
	OBJ_INDEX_DATA *gotobj = NULL;

	if (argument[0] == '\0')
	{
	    stc("Syntax : oolc select <vnum>\n\r", ch);
	    stc("       : oolc select <object name>\n\r", ch);
	    return;
	}

	if (is_number(argument))
 	{
	    if ((obj = get_obj_index(atoi(argument))) == NULL)
	    {
		stc("No such object vnum exists.\n\r", ch);
		return;
	    }
	    if (obj->vnum > ch->zone->max_vnum
		|| obj->vnum < ch->zone->min_vnum)
	    {
		stc("You are not allowed to select that object.\n\r", ch);
		return;
	    }
	    ch->pcdata->curr_vnum_oedit = obj->vnum;
	    stcf(ch, "You are now editing %s, (vnum %d)\n\r", 
		obj->name, obj->vnum);
	    return;
	}

	for (vnum = ch->zone->min_vnum; vnum <= ch->zone->max_vnum; vnum++)
	{
	    obj = get_obj_index( vnum );

	    if (obj == NULL) continue;
	    if (str_prefix(argument, obj->name)) continue;
	    gotobj = obj;
	    break;
	}
	
	if (gotobj == NULL)
	{
	    stc("No such object exists.\n\r", ch);
	    return;
	}

	ch->pcdata->curr_vnum_oedit = gotobj->vnum;

	stcf(ch, "You are now editing '%s' (vnum %d).\n\r", 
	    gotobj->name, gotobj->vnum);
	return;
    }
	
    if (ch->pcdata->curr_vnum_oedit != -1)
	obj = get_obj_index(ch->pcdata->curr_vnum_oedit);

    if (obj == NULL)
    {
	stc("You are not currently editing a valid object.\n\r", ch);
	stc("Use oolc select <vnum> to choose one to edit.\n\r", ch);
	return;
    }
}


void do_molc(CHAR_DATA *ch, char *argument)
{
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char *wholearg = argument;
    MOB_INDEX_DATA *mob = NULL;

    if (ch->in_room->area != ch->zone)
	ch->zone = ch->in_room->area;

    if (ch->in_room == NULL || ch->zone == NULL
	|| ch->in_room->area == NULL)
    {
	send_to_char("You are not currently in a zone or room.\n\r",ch);
	do_prefix(ch, "");
	return;
    }

    if (ch->zone->realm->vnum != ch->build_realm && ch->level != 120)
    {
	send_to_char("You do not have building rights in this area.\n\r",ch);
	do_prefix(ch, "");
	return;
    }

    if (argument[0] == '\0')
    {
	if (ch->prefix != NULL && !str_cmp(ch->prefix, "molc "))
	{
	    send_to_char("#3Ending MobOLC editing mode.#n\n\r",ch);
	    do_prefix(ch, "");
	    REMOVE_BIT(ch->flag5, AFF5_BUILDING);
	    ch->pcdata->curr_vnum_medit = -1;
	}
	else
        {
	    do_prefix(ch, "molc ");
	    send_to_char("#3Welcome to MobOLC editing mode.#n\n\r",ch);
	    SET_BIT(ch->flag5, AFF5_BUILDING);
	}
	return;
    }

    if (ch->prefix != NULL && str_cmp(ch->prefix, "molc "))
    {
	do_prefix(ch, "molc ");
	send_to_char("#3Welcome to MobOLC editing mode#n.\n\r",ch);
    	SET_BIT(ch->flag5, AFF5_BUILDING);
    }

    if (!str_prefix(argument, "help") || argument[0] == '?')
    {
	stc("Syntax: molc [create list edit powers flags attach immune vuln resist description]\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "north")
	|| !str_prefix(argument, "south")
	|| !str_prefix(argument, "east")
	|| !str_prefix(argument, "west")
	|| !str_prefix(argument, "look")
	|| !str_prefix(argument, "up")
	|| !str_prefix(argument, "down"))
    {
	interpret(ch, argument);
	return;
    }

    argument = one_argument(argument, buf1);

    if (!str_prefix(buf1, "create"))
    {
	if (argument[0] == 0)
	{
	    stc("Syntax:  create <new mob name>\n\r", ch);
	    return;
	}

	if (olc_create_mob( ch, argument ) == NULL)
	{
	    stc("Creation operation failed.\n\r", ch);
	    return;
	}
	stc("Mob sucessfully created.\n\r", ch);
	stcf(ch, "Current editing Mob vnum set to %d.\n\r",
	    ch->pcdata->curr_vnum_medit);
	olc_save_zone(ch->zone, ch);
	return;
    }

    if (!str_prefix(buf1, "list"))
    {
	int vnum, count = 0;

	for (vnum = ch->zone->min_vnum; vnum <= ch->zone->max_vnum; vnum++)
	{
	    mob = get_mob_index( vnum );
	    if (mob == NULL || str_prefix(argument, mob->player_name))
		continue;

	    stcf(ch, "Vnum %5d : %s\n\r", vnum, mob->player_name);
	    count++;
	}
	if (count == 0)
	    stc("No mobs were found.\n\r", ch);
	else
	    stcf(ch, "%d mobs matching pattern '%s*' found.\n\r", count, argument);
	return;
    }

    if (!str_prefix(buf1, "select"))
    {
  	int vnum;
	MOB_INDEX_DATA *gotmob = NULL;

	if (argument[0] == '\0')
	{
	    stc("Syntax : molc select <vnum>\n\r", ch);
	    stc("       : molc select <mob name>\n\r", ch);
	    return;
	}

	if (is_number(argument))
 	{
	    if ((mob = get_mob_index(atoi(argument))) == NULL)
	    {
		stc("No such mob vnum exists.\n\r", ch);
		return;
	    }
	    if (mob->vnum > ch->zone->max_vnum
		|| mob->vnum < ch->zone->min_vnum)
	    {
		stc("You are not allowed to select that mob.\n\r", ch);
		return;
	    }
	    ch->pcdata->curr_vnum_medit = mob->vnum;
	    stcf(ch, "You are now editing %s, (vnum %d)\n\r", 
		mob->player_name, mob->vnum);
	    return;
	}

	for (vnum = ch->zone->min_vnum; vnum <= ch->zone->max_vnum; vnum++)
	{
	    mob = get_mob_index( vnum );

	    if (mob == NULL) continue;
	    if (str_prefix(argument, mob->player_name)) continue;
	    gotmob = mob;
	    break;
	}
	
	if (gotmob == NULL)
	{
	    stc("No such mob exists.\n\r", ch);
	    return;
	}
	ch->pcdata->curr_vnum_medit = gotmob->vnum;

	stcf(ch, "You are now editing '%s' (vnum %d).\n\r", 
	    gotmob->player_name, gotmob->vnum);
	return;
    }
	
    if (ch->pcdata->curr_vnum_medit != -1)
	mob = get_mob_index(ch->pcdata->curr_vnum_medit);

    if (mob == NULL)
    {
	stc("You are not currently editing a valid mob.\n\r", ch);
	stc("Use molc select <vnum> to choose one to edit.\n\r", ch);
	return;
    }

    if (!str_prefix(buf1, "edit"))
    {
	menu_init(ch);
	menu_option(ch, 0, "Short description", (void **) &mob->short_descr, MENU_STRING, mob->short_descr);
	menu_option(ch, 1, "Name keywords (lower case)", (void **) &mob->player_name, MENU_STRING, mob->player_name);
	menu_divider(ch, 2);
	menu_option(ch, 4, "Level (1 to 100)", (void **) &mob->level, MENU_SH_INT, mob->level, 1, 100);
	menu_option(ch, 3, "Alignment", (void **) &mob->alignment, MENU_NUMBER, mob->alignment, -1000, 1000);
	sprintf(buf2, "Editing '%s'", mob->short_descr);
	do_menu(ch, "savearea", buf2 );
	return;
    }


    if (!str_prefix(buf1, "description"))
    {
   	char buf[MAX_STRING_LENGTH];

	sprintf(buf, "Enter description for %s.", mob->description);

	do_edit(ch, &mob->description, NULL, buf);
	return;
    }

    if (!str_prefix(buf1, "powers"))
    {
	menu_init_flags( ch, &mob->mobolc_flags2 );
	menu_option( ch, 0, "Poison Bite", NULL, MENU_FLAG, SPEC_POISON_BITE, 0);
	menu_option( ch, 1, "Petrification", NULL, MENU_FLAG, SPEC_PETRIFICATION, 0);
	menu_option( ch, 2, "Flaming Claws", NULL, MENU_FLAG, SPEC_FLAME_CLAW, 0);
	menu_option( ch, 3, "Webs", NULL, MENU_FLAG, SPEC_WEB, 0);
	menu_option( ch, 4, "Fires Spikes", NULL, MENU_FLAG, SPEC_FIRE_SPIKES, 0);
	menu_option( ch, 5, "Everliving", NULL, MENU_FLAG, SPEC_EVERLIVING, 0);
	menu_option( ch, 6, "Splits up upon death", NULL, MENU_FLAG, SPEC_SPLIT, 0);
	menu_option( ch, 7, "Uses claws in combat", NULL, MENU_FLAG, SPEC_COMBAT_CLAWS, 0);
	menu_option( ch, 8, "Uses horns in combat", NULL, MENU_FLAG, SPEC_COMBAT_HORNS, 0);
	menu_option( ch, 9, "Uses fangs in combat", NULL, MENU_FLAG, SPEC_COMBAT_FANGS, 0);
	menu_option( ch, 10, "Uses wings in combat", NULL, MENU_FLAG, SPEC_COMBAT_WINGS, 0);
	menu_option( ch, 11, "Uses tail in combat", NULL, MENU_FLAG, SPEC_COMBAT_TAIL, 0);
	menu_option( ch, 12, "Claws made from steel", NULL, MENU_FLAG, SPEC_STEEL_CLAWS, 0);
	menu_option( ch, 13, "Fire breath", NULL, MENU_FLAG, SPEC_FIRE_BREATH, 0);
	menu_option( ch, 14, "Aging attack", NULL, MENU_FLAG, SPEC_AGE_ATTACK, 0);

	/* When adding more, update add_mobolc_flags in db2.c */

	sprintf(buf2, "Editing powers for '%s'", mob->short_descr);
	do_menu(ch, "savearea", buf2 );
	return;
    }



    if (!str_prefix(buf1, "flags"))
    {
	menu_init_flags( ch, &mob->mobolc_flags );
	menu_option( ch, 0, "Sentinel", NULL, MENU_FLAG, MOBOLC_SENTINEL, 0);
	menu_option( ch, 1, "Scavenger", NULL, MENU_FLAG, MOBOLC_SCAVENGER, 0);
	menu_option( ch, 2, "Aggressive", NULL, MENU_FLAG, MOBOLC_AGGRESSIVE, 0);
	menu_option( ch, 3, "Stays in Area", NULL, MENU_FLAG, MOBOLC_STAY_AREA, 0);
	menu_option( ch, 4, "Wimpy", NULL, MENU_FLAG, MOBOLC_WIMPY, 0);
	menu_option( ch, 5, "Undead", NULL, MENU_FLAG, MOBOLC_UNDEAD, 0);
	menu_option( ch, 6, "Stays Outside", NULL, MENU_FLAG, MOBOLC_OUTDOORS, 0);
	menu_option( ch, 7, "Stays Indoors", NULL, MENU_FLAG, MOBOLC_INDOORS, 0);
	menu_option( ch, 8, "Rideable", NULL, MENU_FLAG, MOBOLC_RIDEABLE, 0);
	menu_option( ch, 9, "Invisibility", NULL, MENU_FLAG, MOBOLC_INVISIBLE, 0);
	menu_option( ch, 10, "Sanctuary", NULL, MENU_FLAG, MOBOLC_SANCTUARY, 0);
	menu_option( ch, 11, "Faerie Fire", NULL, MENU_FLAG, MOBOLC_FAERIE_FIRE, 0);
	menu_option( ch, 12, "Hide", NULL, MENU_FLAG, MOBOLC_HIDE, 0);
	menu_option( ch, 13, "Flying", NULL, MENU_FLAG, MOBOLC_FLYING, 0);
	menu_option( ch, 14, "Haste", NULL, MENU_FLAG, MOBOLC_HASTE, 0);
	menu_option( ch, 15, "Regeneration", NULL, MENU_FLAG, MOBOLC_REGENERATION, 0);
	menu_option( ch, 16, "Pass Door", NULL, MENU_FLAG, MOBOLC_PASS_DOOR, 0);
	menu_option( ch, 17, "Protection from Evil", NULL, MENU_FLAG, MOBOLC_PROTECT_EVIL, 0);
	menu_option( ch, 18, "Protection from Good", NULL, MENU_FLAG, MOBOLC_PROTECT_GOOD, 0);
	menu_option( ch, 19, "Spell Reflection", NULL, MENU_FLAG, MOBOLC_REFLECTION, 0);
	menu_option( ch, 20, "Beast", NULL, MENU_FLAG, MOBOLC_BEAST, 0);
	menu_option( ch, 21, "Eladrin Aura", NULL, MENU_FLAG, MOBOLC_ELAD_AURA, 0);
	menu_option( ch, 22, "Warlock Control", NULL, MENU_FLAG, MOBOLC_WARL_CONTROL, 0);
	menu_option( ch, 22, "Insubstantial", NULL, MENU_FLAG, MOBOLC_INSUBSTANTIAL, 0);

	/* When adding more, update add_mobolc_flags in db2.c */

	sprintf(buf2, "Editing flags for '%s'", mob->short_descr);
	do_menu(ch, "savearea", buf2 );
	return;
    }


    if (!str_prefix(buf1, "immune"))
    {
	menu_init_flags( ch, &mob->imm_flags );
	menu_option( ch, 0, "Charm", NULL, MENU_FLAG, IMM_CHARM, 0);
	menu_option( ch, 1, "Magic", 	NULL, MENU_FLAG, IMM_MAGIC, 0);
	menu_option( ch, 2, "Weapon", 	NULL, MENU_FLAG, IMM_WEAPON,0);
	menu_option( ch, 3, "Bash", 	NULL, MENU_FLAG, IMM_BASH, 0);
	menu_option( ch, 4, "Pierce", 	NULL, MENU_FLAG, IMM_PIERCE, 0);
	menu_option( ch, 5, "Slash", 	NULL, MENU_FLAG, IMM_SLASH, 0);
	menu_option( ch, 6, "Fire", 	NULL, MENU_FLAG, IMM_FIRE, 0);
	menu_option( ch, 7, "Cold", 	NULL, MENU_FLAG, IMM_COLD, 0);
	menu_option( ch, 8, "Lightning",NULL, MENU_FLAG, IMM_LIGHTNING, 0);
	menu_option( ch, 9, "Acid", 	NULL, MENU_FLAG, IMM_ACID, 0);
	menu_option( ch, 10, "Poison", 	NULL, MENU_FLAG, IMM_POISON, 0);
	menu_option( ch, 11, "Negative",NULL, MENU_FLAG, IMM_NEGATIVE, 0);
	menu_option( ch, 12, "Holy", 	NULL, MENU_FLAG, IMM_HOLY, 0);
	menu_option( ch, 13, "Energy", 	NULL, MENU_FLAG, IMM_ENERGY, 0);
	menu_option( ch, 14, "Mental", 	NULL, MENU_FLAG, IMM_MENTAL, 0);
	menu_option( ch, 15, "Disease", NULL, MENU_FLAG, IMM_DISEASE, 0);
	menu_option( ch, 16, "Drowning",NULL, MENU_FLAG, IMM_DROWNING, 0);
	menu_option( ch, 17, "Light", 	NULL, MENU_FLAG, IMM_LIGHT, 0);
	menu_option( ch, 18, "Sound", 	NULL, MENU_FLAG, IMM_SOUND, 0);
	menu_option( ch, 19, "Wood", 	NULL, MENU_FLAG, IMM_WOOD, 0);
	menu_option( ch, 20, "Silver", 	NULL, MENU_FLAG, IMM_SILVER, 0);
	menu_option( ch, 21, "Iron", 	NULL, MENU_FLAG, IMM_IRON, 0);
	sprintf(buf2, "Edit immunity flags for '%s'", mob->short_descr);
	do_menu( ch, "savearea", buf2 );
	return;
    }

    if (!str_prefix(buf1, "vulnerabilities"))
    {
	menu_init_flags( ch, &mob->imm_flags );
	menu_option( ch, 0, "Charm", 	NULL, MENU_FLAG, VULN_CHARM, 0);
	menu_option( ch, 1, "Magic", 	NULL, MENU_FLAG, VULN_MAGIC, 0);
	menu_option( ch, 2, "Weapon", 	NULL, MENU_FLAG, VULN_WEAPON, 0);
	menu_option( ch, 3, "Bash", 	NULL, MENU_FLAG, VULN_BASH, 0);
	menu_option( ch, 4, "Pierce", 	NULL, MENU_FLAG, VULN_PIERCE, 0);
	menu_option( ch, 5, "Slash", 	NULL, MENU_FLAG, VULN_SLASH, 0);
	menu_option( ch, 6, "Fire", 	NULL, MENU_FLAG, VULN_FIRE, 0);
	menu_option( ch, 7, "Cold", 	NULL, MENU_FLAG, VULN_COLD, 0);
	menu_option( ch, 8, "Lightning",NULL, MENU_FLAG, VULN_LIGHTNING, 0);
	menu_option( ch, 9, "Acid", 	NULL, MENU_FLAG, VULN_ACID, 0);
	menu_option( ch, 10, "Poison", 	NULL, MENU_FLAG, VULN_POISON, 0);
	menu_option( ch, 11, "Negative",NULL, MENU_FLAG, VULN_NEGATIVE, 0);
	menu_option( ch, 12, "Holy", 	NULL, MENU_FLAG, VULN_HOLY, 0);
	menu_option( ch, 13, "Energy", 	NULL, MENU_FLAG, VULN_ENERGY, 0);
	menu_option( ch, 14, "Mental", 	NULL, MENU_FLAG, VULN_MENTAL, 0);
	menu_option( ch, 15, "Disease", NULL, MENU_FLAG, VULN_DISEASE, 0);
	menu_option( ch, 16, "Drowning",NULL, MENU_FLAG, VULN_DROWNING, 0);
	menu_option( ch, 17, "Light", 	NULL, MENU_FLAG, VULN_LIGHT, 0);
	menu_option( ch, 18, "Sound", 	NULL, MENU_FLAG, VULN_SOUND, 0);
	menu_option( ch, 19, "Wood", 	NULL, MENU_FLAG, VULN_WOOD, 0);
	menu_option( ch, 20, "Silver", 	NULL, MENU_FLAG, VULN_SILVER, 0);
	menu_option( ch, 21, "Iron", 	NULL, MENU_FLAG, VULN_IRON, 0);
	sprintf(buf2, "Edit immunity flags for '%s'", mob->short_descr);
	do_menu( ch, "savearea", buf2 );
	return;
    }

    if (!str_prefix(buf1, "resistances"))
    {
	menu_init_flags( ch, &mob->res_flags );
	menu_option( ch, 0, "Charm", 	NULL, MENU_FLAG, RES_CHARM, 0);
	menu_option( ch, 1, "Magic", 	NULL, MENU_FLAG, RES_MAGIC, 0);
	menu_option( ch, 2, "Weapon", 	NULL, MENU_FLAG, RES_WEAPON, 0);
	menu_option( ch, 3, "Bash", 	NULL, MENU_FLAG, RES_BASH, 0);
	menu_option( ch, 4, "Pierce", 	NULL, MENU_FLAG, RES_PIERCE, 0);
	menu_option( ch, 5, "Slash", 	NULL, MENU_FLAG, RES_SLASH, 0);
	menu_option( ch, 6, "Fire", 	NULL, MENU_FLAG, RES_FIRE, 0);
	menu_option( ch, 7, "Cold", 	NULL, MENU_FLAG, RES_COLD, 0);
	menu_option( ch, 8, "Lightning",NULL, MENU_FLAG, RES_LIGHTNING, 0);
	menu_option( ch, 9, "Acid", 	NULL, MENU_FLAG, RES_ACID, 0);
	menu_option( ch, 10, "Poison", 	NULL, MENU_FLAG, RES_POISON, 0);
	menu_option( ch, 11, "Negative",NULL, MENU_FLAG, RES_NEGATIVE, 0);
	menu_option( ch, 12, "Holy", 	NULL, MENU_FLAG, RES_HOLY, 0);
	menu_option( ch, 13, "Energy", 	NULL, MENU_FLAG, RES_ENERGY, 0);
	menu_option( ch, 14, "Mental", 	NULL, MENU_FLAG, RES_MENTAL, 0);
	menu_option( ch, 15, "Disease", NULL, MENU_FLAG, RES_DISEASE, 0);
	menu_option( ch, 16, "Drowning",NULL, MENU_FLAG, RES_DROWNING, 0);
	menu_option( ch, 17, "Light", 	NULL, MENU_FLAG, RES_LIGHT, 0);
	menu_option( ch, 18, "Sound", 	NULL, MENU_FLAG, RES_SOUND, 0);
	menu_option( ch, 19, "Wood", 	NULL, MENU_FLAG, RES_WOOD, 0);
	menu_option( ch, 20, "Silver", 	NULL, MENU_FLAG, RES_SILVER, 0);
	menu_option( ch, 21, "Iron", 	NULL, MENU_FLAG, RES_IRON, 0);
	sprintf(buf2, "Edit immunity flags for '%s'", mob->short_descr);
	do_menu( ch, "savearea", buf2 );
	return;
    }

    argument = one_argument(argument, buf2);  

    if (!str_prefix(buf1, "attach"))
    {
	int f, trig = -1;
	MOB_PROG_DATA *prog;

	if (buf2[0] == '\0' || argument[0] == '\0')
   	{
	    stc("Syntax : molc attach <progname> <trigger>\n\r", ch);
	    return;
	}	

	if ( ( prog = get_prog_index(buf2) ) == NULL)
	{
	    stc("Mobprog name not valid, use 'molc prog list' for a list.\n\r", ch);
	    return;
	}

	for (f = 0 ; f < NUM_TRIGS ; f++ )
	{
	    if (trigger[f][0] != '\0' &&
	        !str_prefix(argument, trigger[f])) trig = f;
	}

	if (trig == -1)
 	{
	    stc("Trigger not recognised.\n\r", ch);
	    return;
	}

 	mob->trig[trig] = prog;
	stcf(ch, "Program '%s' attached to the '%s' trigger on %s.\n\r",
	    prog->proc_name, trigger[trig], mob->player_name);
	return;

    }
   

    interpret( ch, wholearg );
}

void do_olc(CHAR_DATA *ch, char *argument)
{
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char *wholearg = argument;

    if (ch->in_room->area != ch->zone)
	ch->zone = ch->in_room->area;

    if (ch->in_room == NULL || ch->zone == NULL
	|| ch->in_room->area == NULL)
    {
	send_to_char("You are not currently in a zone or room.\n\r",ch);
	do_prefix(ch, "");
	return;
    }

    if (ch->in_room->area->clan != CLAN_NONE && ch->in_room->area->clan == ch->clan)
    {
	// can build
    }
    else if ((ch->zone->realm->vnum < 2 || ch->zone->realm->vnum != ch->build_realm) && ch->level != 120)
    {
	send_to_char("You do not have building rights in this area.\n\r",ch);
	do_prefix(ch, "");
	return;
    }


    if (argument[0] == '\0')
    {
	if (ch->prefix != NULL && !str_cmp(ch->prefix, "olc "))
	{
	    send_to_char("Ending OLC editing mode.\n\r",ch);
	    do_prefix(ch, "");
	    REMOVE_BIT(ch->flag5, AFF5_BUILDING);
	}
	else
        {
	    do_prefix(ch, "olc ");
	    send_to_char("Welcome to OLC editing mode.\n\r",ch);
	    SET_BIT(ch->flag5, AFF5_BUILDING);
	}
	return;
    }

    if (!str_prefix(argument, "help") || argument[0] == '\0')
    {
	stc("Syntax: olc [look rname rdesc ndesc rflag rsect door link cobj newclanarea\n\r", ch);
	stc("	          cdesc ehelp nhelp fsect rmes newroom autolook prog]\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "north")
	|| !str_prefix(argument, "south")
	|| !str_prefix(argument, "east")
	|| !str_prefix(argument, "west")
	|| !str_prefix(argument, "look")
	|| !str_prefix(argument, "up")
	|| !str_prefix(argument, "down"))
    {
	interpret(ch, argument);
	return;
    }

    argument = one_argument(argument, buf1);

    if (!str_prefix(buf1, "fsect"))
    {
	if (argument[0] == '\0')
	{
  	    WRITE_STR(ch->in_room->false_sect_type, "None");
	    stc("False sector type removed.\n\r", ch);
	    return;
	}	    

	stc("False room sector set.\n\r", ch);
	WRITE_STR(ch->in_room->false_sect_type, argument);
	return;
    }

    if (!str_cmp(buf1, "newroom") && IS_CREATOR(ch))
    {
	do_newroom( ch, "" );
	return;
    }

    if (!str_prefix(buf1, "cdesc"))
    {
	int dir;
 
	if (argument[0] == '\0')
	{
	    stc("Syntax : cdesc <direction>\n\r", ch);
	    stc(" Use with caution - check direction is correct.\n\r", ch);
	    return;
	}

	dir = get_direction( argument );

	if (dir < 0)
	{
	    send_to_char("That is not a valid direction.\n\r",ch);
	    return;
	}

	if (ch->in_room->exit[dir] == NULL)
	{
	    send_to_char("There isn't an room in that direction.\n\r",ch);
	    return;
	}

	stc("Room description (day and night), name and sector type copied.\n\r", ch);

	if (ch->in_room->description != NULL)
	{
	    WRITE_STR(ch->in_room->exit[dir]->u1.to_room->description,
		    ch->in_room->description);
	}

	if (ch->in_room->name != NULL)
	{
	    WRITE_STR(ch->in_room->exit[dir]->u1.to_room->name,
		    ch->in_room->name);
	}

	if (ch->in_room->night_description != NULL)
	{
	    WRITE_STR(ch->in_room->exit[dir]->u1.to_room->night_description,
		    ch->in_room->night_description);
	}

	ch->in_room->exit[dir]->u1.to_room->sector_type = 
	    ch->in_room->sector_type;

	/* Phew, better save all that! */
	olc_save_zone(ch->zone, ch);
	return;
    }

    if (!str_prefix(buf1, "cobj") && IS_CREATOR(ch))
    {
	int i;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];

	if (argument[0] == '\0')
	{
	    send_to_char("Syntax: olc cobj <object name>\n\r", ch);
	    return;
 	}

	if (argument[1] == '\0')
	{
	    bool found = FALSE;
	    send_to_char("Standard Objects : \n\r", ch);
	    for (i = 0 ; i <= MAX_STDOBJ ; i++)
	    {
	        if (stdobj_data[i].name != NULL
		    && stdobj_data[i].name[0] == argument[0])
	        {
		    sprintf(buf, "%s\n\r", stdobj_data[i].name);
		    send_to_char(buf, ch);
		    found = TRUE;
		}
	    }
	    if (!found)
		send_to_char("None beginning with that letter.\n\r", ch);
	    return;
	}


	for (i = 0 ; i <= MAX_STDOBJ ; i++)
	{
	    if (stdobj_data[i].name != NULL &&
		!str_prefix(argument, stdobj_data[i].name))
	    {
		sprintf(buf, "You create a %s.\n\r", stdobj_data[i].name);
		send_to_char(buf, ch);
		obj = create_std_object( i, 100);	
	 	obj_to_char(obj, ch);
		return;
	    }
	}
	return;
    }

    if (!str_prefix(buf1, "rsect"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("Syntax: olc rsect <sector type>\n\r", ch);
	    return;
	}

	if (!str_prefix(argument, "list"))
	{
	    send_to_char("Sector Types: inside city field forest hills mountains\n\r              deep_water shallow_water underwater air desert tunnel\n\r",ch);
	    return;
	}

	if (!str_prefix(argument, "inside"))
	    ch->in_room->sector_type = SECT_INSIDE;
	else if (!str_prefix(argument, "city"))
	    ch->in_room->sector_type = SECT_CITY;
	else if (!str_prefix(argument, "field"))
	    ch->in_room->sector_type = SECT_FIELD;
	else if (!str_prefix(argument, "forest"))
	    ch->in_room->sector_type = SECT_FOREST;
	else if (!str_prefix(argument, "hills"))
	    ch->in_room->sector_type = SECT_HILLS;
	else if (!str_prefix(argument, "mountain"))
	    ch->in_room->sector_type = SECT_MOUNTAIN;
	else if (!str_prefix(argument, "deep_water"))
	    ch->in_room->sector_type = SECT_WATER_SWIM;
	else if (!str_prefix(argument, "shallow_water"))
	    ch->in_room->sector_type = SECT_WATER_NOSWIM;
	else if (!str_prefix(argument, "underwater"))
	    ch->in_room->sector_type = SECT_UNDERWATER;
	else if (!str_prefix(argument, "air"))
	    ch->in_room->sector_type = SECT_AIR;
	else if (!str_prefix(argument, "desert"))
	    ch->in_room->sector_type = SECT_DESERT;
	else if (!str_prefix(argument, "tunnel"))
	    ch->in_room->sector_type = SECT_NARROW_TUNNEL;
	else
	{
	    send_to_char("No such sector type.\n\r",ch);
	    return;
	}

	send_to_char("Sector type set.\n\r",ch);
	olc_save_zone( ch->zone, ch );
	return;
    }


    if (!str_prefix(buf1, "rname"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("Syntax:  olc rname <room name>\n\r",ch);
	    return;
	}

	free_string(ch->in_room->name);
	ch->in_room->name = str_dup(argument);
	olc_save_zone(ch->zone, ch);
	send_to_char("Room name set.\n\r",ch);
	return;
    }

    if (!str_prefix(buf1, "rflag"))
    {
	int bit;

	if (argument[0] == '\0')
	{
	    send_to_char("Syntax:  olc rflag {show clear list <flag>}\n\r",ch);
	    return;
	}

	if (!str_prefix(argument, "list"))
	{
	    send_to_char("Room Flags:  dark no_mob indoors private safe solitary no_recall cross_water\n\rholy magical small etherial quest\n\r",ch);
	    return;
	}

	if (!str_prefix(argument, "clear"))
	{
	    ch->in_room->room_flags = 0;
	    REMOVE_BIT(ch->in_room->room_flags2, ROOM_QUEST_CENTRE);
	    send_to_char("All bits removed.\n\r",ch);
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	if (!str_prefix(argument, "show"))
	{
	    char buf[1000];
	    buf[0] = '\0';
	    strcat(buf, "Current room flags:  ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_DARK)) 
		strcat(buf, "Dark ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MOB)) 
		strcat(buf, "No_Mob ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_INDOORS)) 
		strcat(buf, "Indoors ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)) 
		strcat(buf, "Private ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)) 
		strcat(buf, "Safe ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)) 
		strcat(buf, "Solitary ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)) 
		strcat(buf, "No_Recall ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_CROSS_WATER)) 
		strcat(buf, "Cross_Water ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_HOLY)) 
		strcat(buf, "Holy ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_MAGICAL)) 
		strcat(buf, "Magical ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_SMALL)) 
		strcat(buf, "Small ");
	    if (IS_SET(ch->in_room->room_flags, ROOM_ETHERIAL)) 
		strcat(buf, "Etherial ");
	    if (IS_SET(ch->in_room->room_flags2, ROOM_QUEST_CENTRE)) 
		strcat(buf, "Quest ");
	    else if (ch->in_room->room_flags == 0)
		strcat(buf, "No flags have been set.");
	    strcat(buf, "\n\r");
	    wrap_to_char(buf, ch);
	    return;
	}

	if (!str_prefix(argument, "quest"))
	{
	    if (ch->level < 120)
	    {
		stc("Only immortals may set this flag.\n\r", ch);
		return;
	    }
	    if (IS_SET(ch->in_room->room_flags2, ROOM_QUEST_CENTRE))
	    {
		send_to_char("Bit removed.\n\r",ch);
		REMOVE_BIT(ch->in_room->room_flags2, ROOM_QUEST_CENTRE);
	    }
	    else
	    {
		send_to_char("Bit set.\n\r", ch);
		SET_BIT(ch->in_room->room_flags2, ROOM_QUEST_CENTRE);
	    }
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	if (!str_prefix(argument, "dark")) bit = ROOM_DARK;
	else if (!str_prefix(argument, "no_mob")) bit = ROOM_NO_MOB;
	else if (!str_prefix(argument, "indoors")) bit = ROOM_INDOORS;
	else if (!str_prefix(argument, "private")) bit = ROOM_PRIVATE;
	else if (!str_prefix(argument, "safe"))
	{
	    if (ch->level < 120)
	    {
		stc("Only immortals may set this flag.\n\r", ch);
		return;
	    }
	    bit = ROOM_SAFE;
	}
	else if (!str_prefix(argument, "solitary"))
	{
	    if (ch->level < 120)
	    {
		stc("Only immortals may set this flag.\n\r", ch);
		return;
	    }
	    bit = ROOM_SOLITARY;
	}
	else if (!str_prefix(argument, "no_recall")) bit = ROOM_NO_RECALL;
	else if (!str_prefix(argument, "cross_water")) bit = ROOM_CROSS_WATER;
	else if (!str_prefix(argument, "holy")) bit = ROOM_HOLY;
	else if (!str_prefix(argument, "magical")) bit = ROOM_MAGICAL;
	else if (!str_prefix(argument, "small")) bit = ROOM_SMALL;
	else if (!str_prefix(argument, "etherial")) bit = ROOM_ETHERIAL;
	else
	{
	    send_to_char("No such flag.\n\r",ch);
	    return;
	}

	if (IS_SET(ch->in_room->room_flags, bit))
	{
	    send_to_char("Bit removed.\n\r",ch);
	    REMOVE_BIT(ch->in_room->room_flags, bit);
	}
	else
	{
	    send_to_char("Bit set.\n\r", ch);
	    SET_BIT(ch->in_room->room_flags, bit);
	}

	olc_save_zone(ch->zone, ch);
	return;
    }

    argument = one_argument(argument, buf2);

    if (!str_prefix(buf1, "newclanarea") && IS_CREATOR(ch))
    {
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	AREA_DATA *area;
	int door;

	if (argument[0] == 0 || buf2[0] == 0)
	{
	    stc("Syntax:  olc newclanarea <clan name> <direction to link>\n\r", ch);
	    return;
	}

	door = get_direction(argument);
	if (door < 0)
	{
	    stc("Syntax:  olc newclanarea <clan name> <direction to link>\n\r", ch);
	    return;
	}
	if (ch->in_room->exit[door] != NULL)
	{
	    stc("There is already an exit in that direction.\n\r", ch);
	    return;
	}
	if (clan_lookup(buf2) < 0)
	{
	    stc("No such clan.\n\r", ch);
	    return;
	}

	area = create_area();
	crash("olc newclanarea : notice me");
	if (area == NULL)
	{
	    stc("Create Area failed.\n\r", ch);
	    return;
	}
	crash("olc newclanarea : notice me again");

	room = olc_create_room_in_area( area );

	crash("olc newclanarea : notice me again again");

	if (room == NULL)
	{
	    stc("Create room in area failed.\n\r", ch);
	    return;
	}

	crash("olc newclanarea - faggots");

	area->clan = clan_lookup(buf2);

	crash("olc newclanarea - just a check");

	exit = alloc_perm( sizeof( *exit ) );
	exit->description = str_dup("Too dark to tell.");
	exit->keyword = str_dup("");
	exit->key = 0;
	exit->exit_info = 0;
	exit->u1.to_room = room;
	ch->in_room->exit[door] = exit;

	door = reverse_direction( door );

	exit = alloc_perm( sizeof( *exit ) );
	exit->description = str_dup("Too dark to tell.");
	exit->keyword = str_dup("");
	exit->key = 0;
	exit->exit_info = 0;
	exit->u1.to_room = ch->in_room;

	room->exit[door] = exit;

	crash("olc newclanarea - just a check 2");

	olc_save_zone(ch->zone, ch);
	crash("olc newclanarea - just a check 5 - 3 sir! - errr .. 3");
	olc_save_zone(room->area, ch);
	crash("olc newclanarea - just a check - bang....");
	stc("New area created.\n\r", ch);
	crash("olc newclanarea - just a check - ....ers and mash");
	return;
    }

    if (!str_prefix(buf1, "prog") && IS_CREATOR(ch))
    {
	MOB_PROG_DATA *mp;
	int ln;

	if (buf2[0] != 0)
	{
	    if (!str_cmp(buf2, "new"))
	    {
		if (argument[0] == 0)
		{
		    stc("What you want the new program called?\n\r", ch);
		    return;
		}
		mp = get_prog_index( argument );
		if (mp != NULL)
		{
		    stc("There is already a program with that name.\n\r", ch);
		    return;
		}
		mp = alloc_perm( sizeof(*mp) );
		mp->next = mob_prog_list;
		mob_prog_list = mp;
		mp->proc_name = str_dup( argument );
		for (ln = 0; ln < MAX_MOB_PROG_LINES; ln++)
		    mp->line[ln] = NULL;
		strcpy(buf2, "edit");
	    }

	    if (!str_cmp(buf2, "edit"))
	    {
		char combuf[1000];
		char otherbuf[1000];

		if (argument[0] == 0)
		{
		    stc("Edit which procedure?\n\r", ch);
		    return;
		}
		mp = get_prog_index( argument );
		if (mp == NULL)
		{
		    stc("A procedure by that name does not exist.\n\r", ch);
		    return;
		}

		for (ln = 0; ln < MAX_MOB_PROG_LINES; ln++)
		{
		    if (mp->line[ln] != NULL)
			strcpy(ch->pcdata->edit[ln], mp->line[ln]);
		    else
			ch->pcdata->edit[ln][0] = 0;
		}

		sprintf(combuf, "%%%%save_prog %s", mp->proc_name);
		bug(combuf, 0);
		sprintf(otherbuf, "Editing procedure '%s'.", mp->proc_name);
		manual_edit(ch, combuf, otherbuf);
		return;
	    }

	    if (!str_cmp(buf2, "remove"))
	    {
		stc("Yeah right.\n\r", ch);
		return;
	    }
	}
	stc("Syntax: prog new    <name> = create a new mob prog.\n\r", ch);
	stc("        prog edit   <name> = edit an existing mob prog.\n\r", ch);
	stc("        prog remove <name> = delete an existing mob prog.\n\r", ch);
	return;
    }

    if (!str_prefix(buf1, "autolook"))
    {
	if (buf2[0] == '\0')
	{
	    stc("Syntax:  autolook +    : to add a new message\n\r", ch);
	    stc("         autolook -    : to remove the last message\n\r", ch);
	    stc("         autolook list : to list the defined messages\n\r", ch);
	    stc("Note in message : % = trans and look, $ = trans only.\n\r", ch);
	    return;
	}

	if (!str_cmp(buf2, "+"))
	{
	    AUTO_LOOK_DATA *ald, *blah;

	    ald                 = alloc_perm( sizeof(*ald) );
            ald->text           = str_dup("New autolook message here.  (CTRL-K) to remove.");
	    ald->next 		= NULL;
            if (ch->in_room->auto_look == NULL)
		ch->in_room->auto_look = ald;
	    else
	    {
		for (blah = ch->in_room->auto_look; blah->next != NULL; blah = blah->next);
		blah->next = ald;
	    }
	    do_edit(ch, &ald->text, NULL, "Enter new autolook message");
	    return;
	}

	if (!str_prefix(buf2, "list"))
	{
	    AUTO_LOOK_DATA *ald;

	    for (ald = ch->in_room->auto_look; ald != NULL; ald = ald->next)
	    {
		stc(ald->text, ch);
		cent_to_char("---------------------------------------------------------", ch);
	    }
	    return;
	}

	if (!str_cmp(buf2, "-"))
	{
	    AUTO_LOOK_DATA *ald, *last = NULL;

	    if (ch->in_room->auto_look == NULL)
	    {
		stc("No auto look messages defined.\n\r", ch);
		return;
	    }

	    for (ald = ch->in_room->auto_look; ald->next != NULL; ald = ald->next)
		last = ald;

	    free_string(ald->text);
	    free_mem( ald, sizeof(*ald) );
	    if (last == NULL)
		ch->in_room->auto_look = NULL;
	    else
		last->next = NULL;

	    stc("Message removed.\n\r", ch);
	    return;
	}

	stc("Syntax:  autolook +    : to add a new message\n\r", ch);
	stc("         autolook -    : to remove the last message\n\r", ch);
	stc("         autolook list : to list the defined messages\n\r", ch);
	stc("Note in message : % = trans and look, $ = trans only.\n\r", ch);	
	return;
    }

    if (!str_prefix(buf1, "rmesd"))
    {
	if (buf2[0] == '\0')
	{
	    stc("Syntax : rmesd I <message> to add a new 'indoors' message.\n\r", ch);
	    stc("         rmesd O <message> to add a new 'outdoors' message.\n\r", ch);
	    stc("         rmesd CI to clear the last 'indoors' message.\n\r", ch);
	    stc("         rmesd CO to clear the last 'outdoors' message.\n\r", ch);
	    stc(" 	 rmesd LI to list the 'indoors' messages.\n\r", ch);
	    stc(" 	 rmesd LO to list the 'outdoors' messages.\n\r", ch);
	    return;
    	}

	if (!str_cmp(buf2, "LI"))
	{
	    AUTO_LOOK_DATA *ald;
	    char buf[MAX_STRING_LENGTH];
	    int count = 1;	

	    if (ch->in_room->area->auto_mesg_indoors == NULL)
	    {
		stc("There are no random indoors messages in this area.\n\r", ch);
		return;
	    }

	    for (ald = ch->in_room->area->auto_mesg_indoors ;
		  ald != NULL ; ald = ald->next)
	    {
	 	sprintf(buf, "%d. %s", count++, ald->text);
		stc(buf, ch);
	    }
	    return;
	}

	if (!str_cmp(buf2, "LO"))
	{
	    AUTO_LOOK_DATA *ald;
	    char buf[MAX_STRING_LENGTH];
	    int count = 1;	

	    if (ch->in_room->area->auto_mesg_outside == NULL)
	    {
		stc("There are no random outdoors messages in this area.\n\r", ch);
		return;
	    }

	    for (ald = ch->in_room->area->auto_mesg_outside ;
		  ald != NULL ; ald = ald->next)
	    {
	 	sprintf(buf, "%d. %s", count++, ald->text);
		stc(buf, ch);
	    }
	    return;
	}

	if (!str_cmp(buf2, "CI"))
	{
	    stc("Last added random indoors message has been cleared.\n\r", ch);
	    stc("Well, not really, cos I cant do it!\n\r", ch);
	    return;
	}

	if (!str_cmp(buf2, "CO"))
	{
	    stc("Last added random outdoors message has been cleared.\n\r", ch);
	    stc("Well, not really, cos I cant do it!\n\r", ch);
	    return;
	}

	if (!str_cmp(buf2, "I"))
	{
	    AUTO_LOOK_DATA *ald;
	    char argbuf[MAX_STRING_LENGTH];

	    if (argument[0] == '\0')
	    {
	        stc("What do you wish to set the random message to?\n\r", ch);
		return;
	    }

	    ald                 = alloc_perm( sizeof(*ald) );
	    sprintf(argbuf, "%s\n\r", argument);
            ald->text           = str_dup(argbuf);
            ald->next = ch->in_room->area->auto_mesg_indoors;
            ch->in_room->area->auto_mesg_indoors = ald;
	    stc("New random indoors message added.\n\r", ch);
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	if (!str_cmp(buf2, "O"))
	{
	    AUTO_LOOK_DATA *ald;
	    char argbuf[MAX_STRING_LENGTH];

	    if (argument[0] == '\0')
	    {
	        stc("What do you wish to set the random message to?\n\r", ch);
		return;
	    }

	    ald                 = alloc_perm( sizeof(*ald) );
	    sprintf(argbuf, "%s\n\r", argument);
            ald->text           = str_dup(argbuf);
            ald->next = ch->in_room->area->auto_mesg_outside;
            ch->in_room->area->auto_mesg_outside = ald;
	    stc("New random outside message added.\n\r", ch);
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	stc("Syntax : rmesd I <message> to add a new 'indoors' message.\n\r", ch);
	stc("         rmesd O <message> to add a new 'outdoors' message.\n\r", ch);
	stc("         rmesd CI to clear the last 'indoors' message.\n\r", ch);
	stc("         rmesd CO to clear the last 'outdoors' message.\n\r", ch);
	stc(" 	  rmesd LI to list the 'indoors' messages.\n\r", ch);
	stc(" 	  rmesd LO to list the 'indoors' messages.\n\r", ch);
	return;
    }

    if (!str_prefix(buf1, "rmesn"))
    {
	if (buf2[0] == '\0')
	{
	    stc("Syntax : rmesn I <message> to add a new 'indoors' message.\n\r", ch);
	    stc("         rmesn O <message> to add a new 'outdoors' message.\n\r", ch);
	    stc("         rmesn CI to clear the last 'indoors' message.\n\r", ch);
	    stc("         rmesn CO to clear the last 'outdoors' message.\n\r", ch);
	    stc(" 	 rmesn LI to list the 'indoors' messages.\n\r", ch);
	    stc(" 	 rmesn LO to list the 'outdoors' messages.\n\r", ch);
	    return;
    	}

	if (!str_cmp(buf2, "LI"))
	{
	    AUTO_LOOK_DATA *ald;
	    char buf[MAX_STRING_LENGTH];
	    int count = 1;	

	    if (ch->in_room->area->auto_mesg_indoors_night == NULL)
	    {
		stc("There are no random night indoors messages in this area.\n\r", ch);
		return;
	    }

	    for (ald = ch->in_room->area->auto_mesg_indoors_night;
		  ald != NULL ; ald = ald->next)
	    {
	 	sprintf(buf, "%d. %s", count++, ald->text);
		stc(buf, ch);
	    }
	    return;
	}

	if (!str_cmp(buf2, "LO"))
	{
	    AUTO_LOOK_DATA *ald;
	    char buf[MAX_STRING_LENGTH];
	    int count = 1;	

	    if (ch->in_room->area->auto_mesg_outside_night == NULL)
	    {
		stc("There are no random outdoors night messages in this area.\n\r", ch);
		return;
	    }

	    for (ald = ch->in_room->area->auto_mesg_outside_night ;
		  ald != NULL ; ald = ald->next)
	    {
	 	sprintf(buf, "%d. %s", count++, ald->text);
		stc(buf, ch);
	    }
	    return;
	}

	if (!str_cmp(buf2, "CI"))
	{
	    stc("Last added random night indoors message has been cleared.\n\r", ch);
	    stc("Well, not really, cos I cant do it!\n\r", ch);
	    return;
	}

	if (!str_cmp(buf2, "CO"))
	{
	    stc("Last added random night outdoors message has been cleared.\n\r", ch);
	    stc("Well, not really, cos I cant do it!\n\r", ch);
	    return;
	}

	if (!str_cmp(buf2, "I"))
	{
	    AUTO_LOOK_DATA *ald;
	    char argbuf[MAX_STRING_LENGTH];

	    if (argument[0] == '\0')
	    {
	        stc("What do you wish to set the random message to?\n\r", ch);
		return;
	    }

	    ald                 = alloc_perm( sizeof(*ald) );
	    sprintf(argbuf, "%s\n\r", argument);
            ald->text           = str_dup(argbuf);
            ald->next = ch->in_room->area->auto_mesg_indoors_night;
            ch->in_room->area->auto_mesg_indoors_night = ald;
	    stc("New random indoors night message added.\n\r", ch);
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	if (!str_cmp(buf2, "O"))
	{
	    AUTO_LOOK_DATA *ald;
	    char argbuf[MAX_STRING_LENGTH];

	    if (argument[0] == '\0')
	    {
	        stc("What do you wish to set the random night message to?\n\r", ch);
		return;
	    }

	    ald                 = alloc_perm( sizeof(*ald) );
	    sprintf(argbuf, "%s\n\r", argument);
            ald->text           = str_dup(argbuf);
            ald->next = ch->in_room->area->auto_mesg_outside_night;
            ch->in_room->area->auto_mesg_outside_night = ald;
	    stc("New random outside night message added.\n\r", ch);
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	stc("Syntax : rmesn I <message> to add a new 'indoors' message.\n\r", ch);
	stc("         rmesn O <message> to add a new 'outdoors' message.\n\r", ch);
	stc("         rmesn CI to clear the last 'indoors' message.\n\r", ch);
	stc("         rmesn CO to clear the last 'outdoors' message.\n\r", ch);
	stc(" 	  rmesn LI to list the 'indoors' messages.\n\r", ch);
	stc(" 	  rmesn LO to list the 'indoors' messages.\n\r", ch);
	return;
    }

    /* Janders helpedit.... its nice oh yes!
	Use ehelp for existing ones....
	 and nhelp for a new help file */

    if (!str_prefix(buf1, "ehelp") && IS_CREATOR(ch))
    {
	HELP_DATA *pHelp, *lHelp = NULL;
   	char buf3[MAX_STRING_LENGTH];
   	char *buf;
        char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
	bool found = FALSE;	
        char arg[MAX_STRING_LENGTH];
        char title[MAX_STRING_LENGTH];
	int cnt=0;
	int level = 666;

	/* first find if its real or not!
            this part handles help a b so that it returns help 'a b' */

	if (ch->level != 120)
	{
	    stc("Only Jander and Myurr can mess around with help files...\n\r", ch);
	    return;
	}

	if (argument[0] == '\0')
	{
	    stc("Syntax : olc ehelp <level> <helpfile>\n\r", ch);
	    stc("	   level is optional - type x for no change.\n\r", ch);
	    stc("	   eg 'ehelp x geluge' to keep level the same.\n\r", ch);
	    return;
	}

	if (is_number(buf2))
	{	
	    if (level < -1 || level > 120)
	    {
	        stc("Level must be between -1 and 120.\n\r", ch);
	        return;
	    }	
	    level = atoi(buf2);
 	}

        argall[0] = '\0';
        while (argument[0] != '\0' )
        {
            argument = one_argument(argument,argone);
            if (argall[0] != '\0')   
                strcat(argall," ");   
            strcat(argall,argone);  
        }

        for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
        {   
            buf = pHelp->keyword;

            while (strlen(buf) > 0)
            {
                buf = split_hash(buf, arg);
     
                if (is_name(argall, arg))
                {
                    cnt += 1;
                    if (cnt == 1)   
                    {
                        lHelp = pHelp;
                        strcpy(title, arg);
                    }
                }

                if (!str_cmp(argall, arg))
                {
                    lHelp = pHelp;
                    strcpy(title, arg);
                    found = TRUE;
                }
            }
        }

  	if (cnt == 0)
  	{
	    stc("There is no help on that word.\n\r", ch);
	    stc("Please use nhelp for a new help file.\n\r", ch);
	    return;
	}

	sprintf(buf3, "Editing help file - %s (Level %d)", lHelp->keyword,
		lHelp->level);
	do_edit(ch, &lHelp->text, NULL, buf3);

	if (level != 666) lHelp->level = level;

	/* need to get rid of all the tildes! - bad! */
	olc_save_helps();
	return;
    }

    if (!str_prefix(buf1, "nhelp") && IS_CREATOR(ch))
    {
	char buf[100];
	HELP_DATA *help, *help2;
  	int level;

	if (ch->level != 120)
	{
	    stc("Only Jander and Myurr can mess around with help files...\n\r", ch);
	    return;
	}

	if (buf1[0] == '\0' || argument[0] == '\0')
	{
	    stc("Syntax : nhelp <level> <keyword list>\n\r", ch);
	    return;
	}

	if (!is_number(buf2))
	{
	    stc("First argument must be a number from -1 to 120.\n\r", ch);
	    return;
	}
 
	level = atoi(buf2);

	if (level < -1 || level > 120)
	{
	    stc("Level must be between -1 and 120.\n\r", ch);
	    return;
	}	
	help = alloc_perm( sizeof(*help) );

	for (help2 = help_first ; help2 != NULL ; help2 = help2->next)
	{
	    if (help2->next == NULL)
	    {
	  	help2->next = help;
	 	help->next = NULL;
	    }
	}

	help->level = level;
	help->keyword = argument;

	sprintf(buf, "Creating help on %s", argument);
	help->text = "Put your help file here!";
	do_edit(ch, &help->text, NULL, buf);
   
	/* Saves all the help files in one big chunk! */
	olc_save_helps();
	return;
    }

    if (!str_prefix(buf1, "door"))
    {
	if (buf2[0] == '\0')
	{
	    send_to_char("Syntax:  olc door {+ - show flags} <direction> <argument>\n\r",ch);
	    return;
	}

	if (!str_prefix(buf2, "+"))
	{
	    int dir;

	    dir = get_direction( argument );
	    if (dir < 0)
	    {
		send_to_char("Syntax:  olc door + <direction>\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[dir] == NULL)
	    {
		send_to_char("There isn't an exit in that direction.\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[dir]->exit_info != 0)
	    {
		send_to_char("There is already a door in that direction.\n\r",ch);
		return;
	    }

	    ch->in_room->exit[dir]->exit_info = EX_ISDOOR;
	    send_to_char("Door created.\n\r",ch);
	    return;
	}

	if (!str_prefix(buf2, "-"))
	{
	    int dir;
	    dir = get_direction( argument );

	    if (dir < 0)
	    {
		send_to_char("Syntax:  olc door - <direction>\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[dir] == NULL)
	    {
		send_to_char("There isn't an exit in that direction.\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[dir]->exit_info == 0)
	    {
		send_to_char("There isn't a door in that direction.\n\r",ch);
		return;
	    }

	    ch->in_room->exit[dir]->exit_info = 0;
	    send_to_char("Door destroyed.\n\r",ch);
	    return;
	}	    

	if (!str_prefix(buf2, "show"))
	{
	    int door;
	    char buf[1000];
	    buf[0] = '\0';
	    door = get_direction( argument );

	    if (door < 0)
	    {
		send_to_char("Syntax:  olc door show <direction>\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[door] == NULL)
	    {
		send_to_char("There isn't an exit in that direction.\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[door]->exit_info == 0)
	    {
		send_to_char("There isn't a door in that direction.\n\r",ch);
		return;
	    }

	    strcat(buf, "Door status:  ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_CLOSED))
		strcat(buf, "Closed ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_LOCKED))
		strcat(buf, "Locked ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_PICKPROOF))
		strcat(buf, "Pick_Proof ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_NOPASS))
		strcat(buf, "No_Pass ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_EASY))
		strcat(buf, "Easy ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_HARD))
		strcat(buf, "Hard ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_INFURIATING))
		strcat(buf, "Infuriating ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_NOCLOSE))
		strcat(buf, "No_Close ");
	    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_NOLOCK))
		strcat(buf, "No_Lock ");
	    if (ch->in_room->exit[door]->exit_info == EX_ISDOOR)
		strcat(buf, "No flags have been set.");
	    strcat(buf, "\n\r");
	    wrap_to_char(buf, ch);
	    return;
	}

	if (!str_prefix(buf2, "flags"))
	{
	    int door, bit;
	    door = get_direction( argument );

	    if (door < 0)
	    {
		send_to_char("Syntax:  olc door show <direction>\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[door] == NULL)
	    {
		send_to_char("There isn't an exit in that direction.\n\r",ch);
		return;
	    }

	    if (ch->in_room->exit[door]->exit_info == 0)
	    {
		send_to_char("There isn't a door in that direction.\n\r",ch);
		return;
	    }


	    argument = one_argument(argument, argument);

	    if (argument[0] == '\0')
	    {
		send_to_char("Set which flag:  closed locked pick_proof no_pass easy hard infuriating\n\rno_close no_lock", ch);
	    	return;
	    }

	    if (!str_prefix(argument, "closed")) bit = EX_CLOSED;
	    else if (!str_prefix(argument, "locked")) bit = EX_LOCKED;
	    else if (!str_prefix(argument, "pick_proof")) bit = EX_PICKPROOF;
	    else if (!str_prefix(argument, "no_pass")) bit = EX_NOPASS;
	    else if (!str_prefix(argument, "easy")) bit = EX_EASY;
	    else if (!str_prefix(argument, "hard")) bit = EX_HARD;
	    else if (!str_prefix(argument, "infuriating")) bit = EX_INFURIATING;
	    else if (!str_prefix(argument, "no_close")) bit = EX_NOCLOSE;
	    else if (!str_prefix(argument, "no_lock")) bit = EX_NOLOCK;
	    else
	    {
	 	send_to_char("No such flag.\n\r",ch);
		return;
	    }

	    if (IS_SET(ch->in_room->exit[door]->exit_info, bit))
	    {
		REMOVE_BIT(ch->in_room->exit[door]->exit_info, bit);
		send_to_char("Bit removed.\n\r",ch);
	    }
	    else
	    {
		SET_BIT(ch->in_room->exit[door]->exit_info, bit);
		send_to_char("Bit set.\n\r",ch);
	    }

	    olc_save_zone(ch->zone, ch);
	    return;
	}

	send_to_char("No such command.\n\r",ch);
    }

    if (!str_prefix(buf1, "rdescription"))
    {
	char buf[200];

	if (ch->in_room == NULL)
	{
	    stc("You are in a NULL room.\n\r", ch);
	    char_to_room(ch, get_room_index(prime_realm, 3054), 0);
	    return;
	}

	sprintf(buf, "Editing %s.",
	    ch->in_room->name);

	do_edit(ch, &ch->in_room->description, "savearea", buf);
	return;
    }

    if (!str_prefix(buf1, "ndescription"))
    {
	char buf[200];

	if (ch->in_room == NULL)
	{
	    stc("You are in a NULL room.\n\r", ch);
	    char_to_room(ch, get_room_index(prime_realm, 3054), 0);
	    return;
	}

	if (!str_prefix(buf2, "clear")
	    && buf2[0] != '\0')
	{
	    stc("Night description cleared.\n\r", ch);
	    ch->in_room->night_description = NULL;
	    olc_save_zone(ch->zone, ch);
	    return;
	}

	sprintf(buf, "Editing %s.",
	    ch->in_room->name);

	if (ch->in_room->night_description == NULL)
 	{
	    WRITE_STR(ch->in_room->night_description,
	     "Edit the night-time description of the room.");
	}

	do_edit(ch, &ch->in_room->night_description, "savearea", buf);
	return;
    }

    if (!str_prefix(buf1, "link"))
    {
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	int door;

	door = get_direction( buf2 );
	if (door < 0)
	{
	    send_to_char("Syntax: olc link <direction> <room number>\n\r",ch);
	    send_to_char("Invalid direction.\n\r",ch);
	    return;
	}

	if (!is_number(argument))
	{
	    send_to_char("Syntax:  olc link <direction> <room number>\n\r",ch);
	    send_to_char("Room number unspecified or not a number.\n\r",ch);
	    return;
	}

	if (ch->in_room->exit[door] != NULL)
	{
	    send_to_char("There is already an exit in that direction.\n\r",ch);
	    return;
	}

	if ((room = get_room_index( char_realm(ch), atoi(argument) )) == NULL)
	{
	    send_to_char("Room does not exist.\n\r",ch);
	    return;
	}

	if (ch->level < 120 && room->area->clan != ch->clan)
	{
	    stc("You may only link to rooms in your clans area.\n\r", ch);
	    return;
	}

	if (room->exit[reverse_direction(door)] != NULL)
	{
	    send_to_char("Target room already has an exit leading back in that direction.\n\r",ch);
	    return;
	}

	exit		    = alloc_perm( sizeof( *exit ) );
    	exit->description   = str_dup("Too dark to tell.");
    	exit->keyword       = str_dup("");
    	exit->exit_info     = 0;
    	exit->key           = 0;  
    	exit->u1.to_room    = room;
	ch->in_room->exit[door] = exit;

	door = reverse_direction( door );

	exit		    = alloc_perm( sizeof( *exit ) );
    	exit->description   = str_dup("Too dark to tell.");
    	exit->keyword       = str_dup("");
    	exit->exit_info     = 0;
    	exit->key           = 0;  
    	exit->u1.to_room    = ch->in_room;
	room->exit[door] = exit;

	olc_save_zone( ch->zone, ch );
	olc_save_zone( room->area, ch );
	send_to_char("You link the two rooms together.\n\r",ch);
	return;
    }

    interpret( ch, wholearg );
}

void do_save_prog(CHAR_DATA *ch, char *argument)
{
    MOB_PROG_DATA *mp;
    char buf[4000];
    int ln, cnt = 0, cp, yp;

    if (IS_NPC(ch)) return;

    argument = one_argument(argument, buf);

    mp = get_prog_index( argument );
    if (mp == NULL)
	return;

    yp = 0;

    for (ln = 0; ln < MAX_EDITOR_LINES; ln++)
    {
	if (ch->pcdata->edit[ln] == NULL || ch->pcdata->edit[ln][0] == 0)
	{
	    free_string(mp->line[ln]);
	    mp->line[ln] = NULL;
	    cnt += 1;
	    continue;
	}

	for (cp = 0; cp < cnt; cp++)
	{
	    WRITE_STR(mp->line[yp], "\n\r");
	    yp += 1;
	}
	cnt = 0;
	WRITE_STR(mp->line[yp], ch->pcdata->edit[ln]);
	yp += 1;
    }

    validate_prog(ch, mp);
    olc_save_mob_progs();
}

void do_build( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vict;

    if (argument[0] == '\0')
    {
	send_to_char("Build {here} {<char>}\n\r",ch);
	return;
    }

    if (!str_cmp(argument, "here"))
    {
	ch->build_realm = ch->zone->realm->vnum;
	send_to_char("You may now build here.\n\r",ch);
	return;
    }

    if (( vict = get_char_world(ch, argument) ) == NULL)
    {
	send_to_char("They are not here.\n\r",ch);
	return;
    }

    vict->build_realm = vict->zone->realm->vnum;
    send_to_char("They can now build in the area they are currently in.\n\r",ch);
    send_to_char("You can now build here.\n\r",vict);
}

void do_dig(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *rm;
    int dir;

    if (argument[0] == '\0')
    {
	send_to_char("Syntax: Dig <direction>\n\r",ch);
	return;
    }

    if (ch->in_room->area->clan != CLAN_NONE && ch->in_room->area->clan == ch->clan)
    {
	// can build
    }
    else if ((ch->zone->realm->vnum < 2 || ch->zone->realm->vnum != ch->build_realm) && ch->level != 120)
    {
	send_to_char("You do not have building rights in this area.\n\r",ch);
	return;
    }

    dir = get_direction(argument);
    if (dir < 0)
    {
	send_to_char("That isn't a direction.\n\r",ch);
	return;
    }

    if (ch->in_room->exit[dir] != NULL)
    {
	send_to_char("There is already a room in that direction.\n\r",ch);
	return;
    }

    rm = olc_create_room(ch->in_room, dir);

    if (rm == NULL)
    {
	send_to_char("You try to tunnel into the rock, but it is too hard.\n\r",ch);
	return;
    }

    send_to_char("You tunnel into the rock, creating a new cavern.\n\r",ch);
    olc_save_zone( rm->area, ch );
}



void do_newroom(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *rm;

    if (ch->zone->realm->vnum != ch->build_realm)
    {
	send_to_char("You are not allowed to dig in this area.\n\r",ch);
	return;
    }

    rm = olc_create_room(ch->in_room, -1);

    if (rm == NULL)
    {
	send_to_char("You try to tunnel into the rock, but it is too hard.\n\r",ch);
	return;
    }

    send_to_char("You tunnel into the rock, creating a new cavern.\n\r",ch);
    send_to_char("You are transported there by unseen powers.\n\r",ch);
    char_from_room( ch );
    char_to_room( ch, rm, 0 );
    olc_save_zone( rm->area, ch );
}

MOB_INDEX_DATA * olc_create_mob(CHAR_DATA *ch, char *argument)
{
    MOB_INDEX_DATA *mob;
    int vnum;

    vnum = find_free_mob_vnum( ch->in_room->area );

    if (vnum == -1)
	return NULL;

    if (ch != NULL
	&& !IS_NPC(ch)) ch->pcdata->curr_vnum_medit = vnum;

    mob 		= alloc_perm( sizeof(*mob) );
    mob->vnum 		= vnum;
    mob->area		= ch->zone;
    mob->new_format	= TRUE;
    mob->player_name	= str_dup(argument);
    mob->short_descr	= str_dup("A mob with no short name");
    mob->long_descr	= str_dup("A mob with no room description stands here, tapping his foot.\n\r");
    mob->description	= str_dup("This mob looks annoyed as he has no description.\n\r");
    mob->race		= race_lookup("unique");
    mob->act		= ACT_IS_NPC;
    mob->affected_by	= 0;
    mob->alignment	= 0;
    mob->group		= vnum;
    mob->level		= 1;
    mob->hitroll	= 0;

    mob->hit[DICE_NUMBER] 	= 1;
    mob->hit[DICE_TYPE]		= 1;
    mob->hit[DICE_BONUS]	= 1;
    mob->damage[DICE_NUMBER]	= 1;
    mob->damage[DICE_TYPE]	= 1;
    mob->damage[DICE_BONUS]	= 1;

    mob->dam_type	= attack_lookup("hit");
    mob->ac[AC_PIERCE]	= 10;
    mob->ac[AC_BASH]	= 10;
    mob->ac[AC_SLASH]	= 10;
    mob->ac[AC_EXOTIC]	= 10;
    mob->off_flags	= 0;
    mob->imm_flags	= 0;
    mob->res_flags	= 0;
    mob->vuln_flags	= 0;
    mob->start_pos	= POS_STANDING;
    mob->default_pos	= POS_STANDING;
    mob->sex		= 1;
    mob->wealth		= 0;
    mob->form		= 0;
    mob->parts		= 0;
    mob->size		= SIZE_MEDIUM;
    mob->material	= str_dup("flesh");

    add_mob( mob );

    return mob;
}

OBJ_INDEX_DATA * olc_create_obj(CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA *obj;
    int vnum;

    vnum = find_free_obj_vnum( ch->in_room->area );

    if (vnum == -1)
	return NULL;

    if (ch != NULL
	&& !IS_NPC(ch)) ch->pcdata->curr_vnum_oedit = vnum;

    obj 		= alloc_perm( sizeof(*obj) );
    obj->vnum 		= vnum;
    obj->new_format	= TRUE;
    obj->name		= str_dup(argument);
    obj->short_descr	= str_dup("An object with no short name");
    obj->description	= str_dup("An unnamed object lies here.\n\r");

    obj->material	= str_dup("lego");
    obj->perm_spell	= -1;
    obj->item_type	= 0;
    obj->extra_flags 	= 0;
    obj->extra_flags2 	= 0;
    obj->wear_flags	= 0;
    obj->reset_num	= 0;

    obj->value[1] 	= 0;
    obj->value[2] 	= 0;
    obj->value[3] 	= 0;
    obj->value[4] 	= 0;
    obj->value[5] 	= 0;

    obj->level 		= 0;
    obj->weight		= 10;
    obj->cost		= 10;
    obj->spec_proc	= 0;
    obj->condition	= 100;
    obj->artifact	= 0;

    add_obj( obj );

    return obj;
}

char *free_file_name()
{
    FILE *fp;
    bool unique = FALSE;
    char buf[10];

    sprintf(buf, "clan%d%d%d%d.are", dice(1, 9), dice(1, 9), dice(1, 9), dice(1, 9));
    fp = fopen(buf, "r");
    if (fp == NULL) unique = TRUE;
    else fclose(fp);
    if (!unique)
	return free_file_name();
    else
	return str_dup(buf);
}

int free_min_vnum()
{
    AREA_DATA *area;
    int vnum;
    bool found = FALSE;

    for (vnum = 50; vnum < 100000; vnum += 100)
    {
	found = FALSE;
	for (area = area_first; area != NULL; area = area->next)
	{
	    if (vnum > area->min_vnum && vnum < area->max_vnum )
		found = TRUE;
	}
	if (!found)
	    return vnum - 50;
    }
    return -1;
}


AREA_DATA *create_area( )
{
    AREA_DATA *pArea;
    FILE *fp, *fout;
    char *word;

    if (free_min_vnum() < 0)
	return NULL;

    crash("Create area - breasts");

    pArea				= alloc_perm( sizeof(*pArea) );
    pArea->reset_first			= NULL;
    pArea->reset_last			= NULL;
    pArea->file_name			= free_file_name();
    pArea->credits			= str_dup("<---> Autogen Area by Myurr   ");
    pArea->name				= str_dup("Autogen");
    pArea->min_vnum			= free_min_vnum();
    pArea->max_vnum			= pArea->min_vnum + 99;
    bug("min vnum %d", pArea->min_vnum);
    bug("max vnum %d", pArea->max_vnum);
    pArea->age				= 15;
    pArea->nplayer			= 0;
    pArea->empty			= FALSE;
    pArea->repop_status			= REPOP_NEEDED;
    pArea->auto_mesg_indoors		= NULL;
    pArea->auto_mesg_outside		= NULL;
    pArea->auto_mesg_indoors_night	= NULL;
    pArea->auto_mesg_outside_night	= NULL;
    pArea->clan				= CLAN_NONE;
    pArea->realm			= get_realm("The Prime Material Plane");

    crash("Create area - grated nipples");

    if ( area_first == NULL)
	area_first = pArea;
    if ( area_last != NULL )
	area_last->next = pArea;
    area_last = pArea;
    pArea->next = NULL;

    crash("Create area - tits made from cheese");

    if ( ( fp = fopen( AREA_LIST, "r" ) ) == NULL )
    {
	perror( AREA_LIST );
	exit( 1 );
    }

    if ( ( fout = fopen( "area.lst.tmp", "w" ) ) == NULL)
    {
	perror( AREA_LIST );
	exit( 1 );
    }

    crash("Create area - arseholes and elbows");

    for ( ; ; )
    {
	word = fread_word( fp );
	if (word[0] == '$')
	    break;
	fprintf( fout, "%s\n", word );
    }

    fprintf( fout, "%s\n$\n", pArea->file_name );

    fclose(  fp  );
    fclose( fout );

    crash("Create area - get off my tits");

    rename( "area.lst.tmp", AREA_LIST );

    crash("Create area - ok!  I'm off your tits already");

    return pArea;
}


ROOM_INDEX_DATA * olc_create_room(ROOM_INDEX_DATA *room, int dir)
{
    ROOM_INDEX_DATA *rm;
    int vnum, door;
    EXIT_DATA *exit;

    if (dir != -1 && room->exit[dir] != NULL)
	return NULL;

    vnum = find_free_room_vnum( room->area );

    if (vnum == -1)
	return NULL;

    rm 			= alloc_perm( sizeof(*rm) );
    rm->owner		= NULL;
    rm->people		= NULL;
    rm->contents	= NULL;
    rm->extra_descr	= NULL;
    rm->false_sect_type = NULL;
    rm->area		= room->area;
    rm->vnum		= vnum;
    rm->name		= str_dup("A large cavern.");
    rm->gore		= 0;
    rm->description     = str_dup("Put the new room description here! (Ctrl-K to delete a line)");
    rm->room_flags	= 0;
    rm->room_flags2	= 0;
    rm->sector_type	= SECT_INSIDE;
    rm->light		= 0;
    for (door = 0; door <= 5; door ++)
	rm->exit[door] = NULL;
    rm->heal_rate 	= 100;

    if (dir != -1)
    {
    	exit		= alloc_perm( sizeof(*exit) );
    	exit->description	= str_dup("Too dark to tell.");
    	exit->keyword	= str_dup("");
    	exit->exit_info	= 0;
    	exit->key		= 0;
    	exit->u1.to_room	= room;

    	dir = reverse_direction( dir );

    	rm->exit[dir]	= exit;
    	rm->old_exit[dir]	= exit;
    
    	exit		= alloc_perm( sizeof(*exit) );
    	exit->description	= str_dup("Too dark to tell.");
    	exit->keyword	= str_dup("");
    	exit->exit_info	= 0;
    	exit->key		= 0;
    	exit->u1.to_room	= rm;

    	dir = reverse_direction( dir );

    	room->exit[dir]	= exit;
    	room->old_exit[dir]	= exit;
    }

    add_room( rm ); 

    return rm;
}


ROOM_INDEX_DATA * olc_create_room_in_area(AREA_DATA *area)
{
    ROOM_INDEX_DATA *rm;
    int vnum, door;

    crash("create_room_in_clan:  Looking for someone?  Found someone I'd say.");

    vnum = find_free_room_vnum( area );

    crash("create_room_in_clan:  Storm troopers?  Here?  Oh no!  I've been shot!");

    if (vnum == -1)
	return NULL;

    crash("create_room_in_clan:  Curse my metal skin R2");

    rm 			= alloc_perm( sizeof(*rm) );
    rm->owner		= NULL;
    rm->people		= NULL;
    rm->contents	= NULL;
    rm->extra_descr	= NULL;
    rm->false_sect_type = NULL;
    rm->area		= area;
    rm->vnum		= vnum;
    rm->name		= str_dup("A large cavern.");
    rm->gore		= 0;
    rm->description     = str_dup("Put the new room description here! (Ctrl-K to delete a line)");
    rm->room_flags	= 0;
    rm->room_flags2	= 0;
    rm->sector_type	= SECT_INSIDE;
    rm->light		= 0;
    for (door = 0; door <= 5; door ++)
	rm->exit[door] = NULL;
    rm->heal_rate 	= 100;

    crash("create_room_in_area:  If you don't mind sir, I'd like to shut down for a while");

    add_room( rm ); 

    crash("create_room_in_area: Sure, go ahead");

    return rm;
}

int get_direction(char *argument)
{
    if (argument[0] == '\0')
	return -1;
    if (!str_prefix(argument, "north"))
	return 0;
    else if (!str_prefix(argument, "east"))
	return 1;
    else if (!str_prefix(argument, "south"))
	return 2;
    else if (!str_prefix(argument, "west"))
	return 3;
    else if (!str_prefix(argument, "up"))
	return 4;
    else if (!str_prefix(argument, "down"))
	return 5;

    return -1;
}

int reverse_direction(int dir)
{
    if (dir == 0)
	return 2;
    else if (dir == 1)
	return 3;
    else if (dir == 2)
	return 0;
    else if (dir == 3)
	return 1;
    else if (dir == 4)
	return 5;
    else if (dir == 5)
	return 4;

    return -1;
}

int find_free_room_vnum(AREA_DATA *area)
{
    int vn, rm = -1;
    bool found = FALSE;

    if (area == NULL)
    {
	bug("Null Area passed to find_free_room_vnum.",0);
	return -1;
    }

    for (vn = (area->min_vnum > 0) ? area->min_vnum : 1; vn <= area->max_vnum; vn++)
    {
	if (get_room_index( area->realm, vn ) == NULL)
	{
	    rm = vn;
	    found = TRUE;
	    break;
	}
    }

    if (!found) return -1;
    return rm;
}

int find_free_mob_vnum(AREA_DATA *area)
{
    int vn, rm = -1;
    bool found = FALSE;

    if (area == NULL)
    {
	bug("Null Area passed to find_free_mob_vnum.",0);
	return -1;
    }

    for (vn = area->min_vnum; vn <= area->max_vnum; vn++)
    {
	if (get_mob_index( vn ) == NULL)
	{
	    rm = vn;
	    found = TRUE;
	    break;
	}
    }

    if (!found) return -1;
    return rm;
}

int find_free_obj_vnum(AREA_DATA *area)
{
    int vn, rm = -1;
    bool found = FALSE;

    if (area == NULL)
    {
	bug("Null Area passed to find_free_obj_vnum.",0);
	return -1;
    }

    for (vn = area->min_vnum; vn <= area->max_vnum; vn++)
    {
	if (get_obj_index( vn ) == NULL)
	{
	    rm = vn;
	    found = TRUE;
	    break;
	}
    }

    if (!found) return -1;
    return rm;
}

void do_olc_save_area(CHAR_DATA *ch, char *argument)
{
    send_to_char("Attempting to save your current area.\n\r",ch);

    if (ch->zone == NULL)
    {
	send_to_char("Hey dumb arse!  You ain't in an area.\n\r",ch);
	return;
    }

    stc("Hokey dokey its done.\n\r", ch);

    olc_save_zone( ch->zone, ch );
}

void olc_save_zone(AREA_DATA *zone, CHAR_DATA *ch)
{
    FILE *f;
    sh_int vn;
    ROOM_INDEX_DATA *location;
    OBJ_INDEX_DATA *obj;
    MOB_INDEX_DATA *mob;
    RESET_DATA *reset;
    SHOP_DATA *shop;
    char fname[100], fname2[100];

    crash("olc save zone");

    if (zone->repop_status == REPOP_HOUSING_DONE)
	sprintf(fname, "housing/housing.are");
    else
	sprintf(fname, "%s", zone->file_name);

    sprintf(fname2, "cp %s backup/%s >> shell.out", fname, fname);
    system( fname2 );

    if ((f = fopen(fname, "w")) == NULL)
    {
	bug("Error opening file area.out",0);
	return;
    }

    fprintf( f, "#AREA\n");

    olc_save_area_header( f, zone );

    fprintf( f, "#OLCMOB\n");
    for (vn = zone->min_vnum; vn <= zone->max_vnum; vn++)
    {
	if ( ( mob = get_mob_index( vn ) ) != NULL )
	    olc_save_mob( f, mob );
    }
    fprintf( f, "#0\n\n");

    fprintf( f, "#OLCOBJ\n");
    for (vn = zone->min_vnum; vn <= zone->max_vnum; vn++)
    {
	if ( ( obj = get_obj_index( vn ) ) != NULL )
	    olc_save_object( f, obj );
    }
    fprintf( f, "#0\n\n");


    fprintf( f, "#ROOMS\n");
    for (vn = zone->min_vnum; vn <= zone->max_vnum; vn++)
    {
	if ( ( location = get_room_index( zone->realm, vn ) ) != NULL )
	    olc_save_room( f, location );
    }
    fprintf( f, "#0\n\n");

    fprintf( f, "#RESETS\n");
    for (reset = zone->reset_first; reset != NULL; reset = reset->next)
    {
	olc_save_reset( f, reset );
    }
    fprintf( f, "S\n\n");

    for (shop = zone->shop_list; shop != NULL; shop = shop->next)
	olc_save_shop( f, shop );

    fprintf( f, "#$\n");
    fclose( f );
}

void olc_save_shop( FILE *fp, SHOP_DATA *shop )
{
    SUPPLIER_DATA *sup;

    fprintf(fp, "#SHOP\%d %d %d %d\n%d %d %ld\n",
	shop->realm, shop->vnum, shop->shop_room, shop->restart_cash,
	shop->time_open, shop->time_close, shop->flags);

    fprintf(fp, "%s~\n%s~\n%s~\n%s~\n%d %d %s~\n",
	shop->route_home, shop->route_shop, shop->route_to_stores,
	shop->route_from_stores, shop->profit_buying,
	shop->profit_selling, shop->offender);

    for (sup = shop->supply_list; sup != NULL; sup = sup->next)
    {
	fprintf(fp, "+%s~\n%d %d %d %d %d\n",
	    sup->name, sup->vnum_supplied, sup->price, sup->varience,
	    sup->number, sup->frequency);
    }

    fprintf(fp, "-\n\n");
}


void do_olc_save_mob_progs( CHAR_DATA *ch, char *argument)
{
    olc_save_mob_progs();
    olc_save_helps();
    stc("Sure thang mate.\n\r", ch);
}

void olc_save_mob_progs( void )
{
    MOB_PROG_DATA *mp;
    FILE *f;
    int ln;

    if ((f = fopen("mobprog.are", "w")) == NULL)
    {
	bug("Error opening file mobprog.are",0);
	return;
    }

    fprintf(f, "#MOBPROG\n\n");

    for (mp = mob_prog_list; mp != NULL; mp = mp->next)
    {
	fprintf(f, "#%s\n{\n", mp->proc_name);
	for (ln = 0; ln < MAX_MOB_PROG_LINES; ln++)
	{
	    if (mp->line[ln] != NULL && mp->line[ln][0] != 0)
		fprintf(f, "    %s\n", mp->line[ln]);
	}
	fprintf(f, "}\n\n");
    }

    fprintf(f, "#0\n#$\n");
    fclose( f );
}

void olc_save_helps( void )
{
    HELP_DATA *help;
    FILE *f;
    char buf[MAX_STRING_LENGTH];

    if ((f = fopen("helps.are", "w")) == NULL)
    {
	bug("Error opening file helps.are",0);
	return;
    }

    bug("Olc save helps!", 0);

    system("cp helps.are backup/helps.are >> shell.out");

    fprintf(f, "#HELPS\n\n");

    for (help = help_first; help != NULL ; help = help->next)
    {
        copy_description( buf, help->text ); 
	fprintf(f, "%d %s~\n%s~\n", help->level, help->keyword,
		buf);
    }

    fprintf(f, "0 $~\n\n#$\n");

    fclose( f );
}

void olc_save_area_header( FILE *fp, AREA_DATA *pArea )
{
    AUTO_LOOK_DATA *ald;

    fprintf(fp, "%s~\n%s~\n%s~\n",
	pArea->file_name,
	pArea->name,
	pArea->credits);

    fprintf(fp, "%d %d\n\n",
	pArea->min_vnum,
	pArea->max_vnum);

    for (ald = pArea->auto_mesg_indoors_night ; ald != NULL ; ald = ald->next)
	fprintf(fp, "i %s~\n", ald->text);

    for (ald = pArea->auto_mesg_outside_night ; ald != NULL ; ald = ald->next)
	fprintf(fp, "o %s~\n", ald->text);

    for (ald = pArea->auto_mesg_indoors ; ald != NULL ; ald = ald->next)
	fprintf(fp, "I %s~\n", ald->text);

    for (ald = pArea->auto_mesg_outside ; ald != NULL ; ald = ald->next)
	fprintf(fp, "O %s~\n", ald->text);

    fprintf(fp, "C %s~\n", clan_table[pArea->clan].name);

    fprintf(fp, "R %s~\n", pArea->realm->name);
}

void olc_save_mob( FILE *fp, MOB_INDEX_DATA *mob )
{
    int trig;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    copy_description(buf, mob->description);
    copy_description(buf2, mob->long_descr);

    fprintf(fp, "#%d\n%s~\n%s~\n%s~\n%s~\n",
	mob->vnum, mob->player_name, mob->short_descr, buf2,
	buf);
    fprintf(fp, "%d %ld %ld %d %d %d\n%d %d %d\n%d %d %d\n",
	mob->group, mob->act, mob->affected_by, mob->alignment,
	mob->level, mob->hitroll, mob->hit[0], mob->hit[1], mob->hit[2],
	mob->damage[0],
	mob->damage[1], mob->damage[2]);
    fprintf(fp, "%d %d %d %d %d\n%ld %ld %ld %ld\n%d %d %d %d %ld %ld %ld\n%ld %d\n%s~\n%d %d %d\n\n",
	mob->ac[0], mob->ac[1], mob->ac[2], mob->ac[3], mob->dam_type,
	mob->off_flags, mob->imm_flags, mob->res_flags, mob->vuln_flags,
	mob->start_pos, mob->default_pos, mob->sex, mob->race, mob->wealth,
	mob->form, mob->parts, mob->mob_spec, mob->size, mob->material,
	mob->body_form, mob->secondaryrace, mob->tertiaryrace);

    for (trig = 0; trig < NUM_TRIGS ; trig++)
	if (mob->trig[trig] != NULL)
	    fprintf(fp, "M %d %s\n", trig, mob->trig[trig]->proc_name);

    fprintf(fp, "O %ld %ld\n",
	mob->mobolc_flags, mob->mobolc_flags2);
}

void olc_save_reset( FILE *fp, RESET_DATA *reset )
{
    fprintf( fp, "%c 0 %d %d",
	reset->command,
	reset->arg1,
	reset->arg2);

    if (reset->command != 'G' && reset->command != 'R')
	fprintf(fp, " %d", reset->arg3);

    fprintf(fp, " %d\n", reset->arg4);
}

void copy_description( char *to, char *from )
{
    int i, j = 0;

    if (from == NULL)
    {
	bug("copy_description: From == NULL",0);
	return;
    }

    if (to == NULL)
    {
	bug("copy_description: To == NULL",0);
	return;
    }

    for (i = 0; from[i] != '\0'; i++)
    {
	if (from[i] != 13)
	    to[j++] = from[i];
    }

    to[j] = '\0';
}

void olc_save_object( FILE *fp, OBJ_INDEX_DATA *obj)
{
    EXTRA_DESCR_DATA *ed;
    char buf[MAX_STRING_LENGTH];

    fprintf(fp, "#%d\n", obj->vnum);

    copy_description( buf, obj->short_descr );
    fprintf(fp, "%s~\n%s~\n", obj->name, buf);

    copy_description( buf, obj->description );
    fprintf(fp, "%s~\n%s~\n", buf, obj->material); 

    fprintf(fp, "%d %d %d\n%d %d %d %d %d\n%d %d %d %d\n",
	obj->item_type,
	obj->extra_flags,
	obj->wear_flags,
	obj->value[1],
	obj->value[2],
	obj->value[3],
	obj->value[4],
	obj->value[5],
	obj->level,
	obj->weight,
	obj->cost,
	obj->condition);

    if (obj->spec_proc != 0) 
	fprintf( fp, "Z %d\n", obj->spec_proc );    

    if (obj->perm_spell != -1) 
	fprintf( fp, "S %d\n", obj->perm_spell );    

    for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
    {
	copy_description( buf, ed->description );
	fprintf(fp, "E %s~\n%s~\n",
	    ed->keyword,
	    buf);
    }

    fprintf(fp, "\n");
}

void olc_save_room( FILE *fp, ROOM_INDEX_DATA *r )
{
    EXTRA_DESCR_DATA *ed;
    int door;
    char buf[MAX_STRING_LENGTH];
    AUTO_LOOK_DATA *ald;

    buf[0] = '\0';

    copy_description( buf, r->description );

    fprintf(fp, "#%d\n%s~\n%s~\n%d ",
	r->vnum,
	r->name,
	buf,
	0);

    fprintf(fp, "%d %d\n",
	r->room_flags,
	r->sector_type);

    if (r->heal_rate != 100)
    	fprintf(fp, "H %d\n", r->heal_rate);

    /* Autolook data saving */
    for (ald = r->auto_look ; ald != NULL ; ald = ald->next )
	fprintf(fp, "A %s~\n", ald->text);

    /* False sector type saving */
    if (r->false_sect_type != NULL)
	fprintf(fp, "F %s~\n", r->false_sect_type);

    /* Night descriptions saving */
    if (r->night_description != NULL)
    {
        copy_description( buf, r->night_description);
	fprintf(fp, "N %s~\n", buf);
    }

    for (ed = r->extra_descr; ed != NULL; ed = ed->next)
    {
    	copy_description( buf, ed->description );
	fprintf(fp, "E\n%s~\n%s~\n",
	    ed->keyword,
	    buf);
    }

    for (door = 0; door < 6; door++)
    {
	if (r->exit[door] != NULL && r->exit[door]->u1.to_room != NULL)
	{
	    int n = 0;
	    copy_description( buf, r->exit[door]->description );

	    fprintf(fp, "D%d\n%s~\n%s~\n",
		door,
		buf,
		r->exit[door]->keyword);

	    if (IS_SET(r->exit[door]->exit_info, EX_ISDOOR))
	    	n = 1;
	    if (IS_SET(r->exit[door]->exit_info, EX_PICKPROOF))
	    	n += 1;
	    if (IS_SET(r->exit[door]->exit_info, EX_NOPASS))
	    	n += 2;

	    fprintf(fp, "%d %d %d\n",
		n,
		r->exit[door]->key,
		r->exit[door]->u1.to_room->vnum);
	}
    }

    fprintf(fp, "S\n");
}

void do_housedescription(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch) || ch->in_room == NULL || ch->pcdata == NULL
	|| ch->pcdata->home != ch->in_room->vnum)
    {
	stc("You must be in your new home to use this command.\n\r", ch);
	return;
    }

    if (rb_room == ch->in_room)
    {
	stc("You cannot use this command while your house is under construction.\n\r", ch);
	return;
    }

    do_edit(ch, &ch->in_room->description, NULL, NULL);
}
