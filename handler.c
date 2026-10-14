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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return      );
DECLARE_DO_FUN(do_look        );
DECLARE_DO_FUN(do_say         );



/*
 * Local functions.
 */
void    affect_modify   args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
bool    is_dagger	args( (OBJ_DATA *obj) );
bool    can_regen_body_parts args ( (CHAR_DATA *ch) );
void	make_blood	args( (CHAR_DATA *ch) );

char *split_hash (char *argument, char *arg_first)
{
    char cEnd;
        
    while ( isspace(*argument) )
        argument++;
            
    cEnd = '#';

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first = LOWER(*argument);
        arg_first++;
        argument++;
    }
    *arg_first = '\0';
        
    while ( isspace(*argument) )
        argument++;
            
    return argument;
}

char *split_semicolon (char *argument, char *arg_first)
{
    char cEnd;
        
    while ( isspace(*argument) )
        argument++;
            
    cEnd = ';';

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd)
        {
            argument++;
	    if (*argument != cEnd)
	        break;
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';
        
    while ( isspace(*argument) )
        argument++;
            
    return argument;
}


char *split_misc (char *argument, char *arg_first, char cEnd)
{
    while ( isspace(*argument) )
        argument++;
            
    while ( *argument != '\0' )
    {
        if ( *argument == cEnd)
        {
            argument++;
	    if (*argument != cEnd)
	        break;
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';
        
    while ( isspace(*argument) )
        argument++;
            
    return argument;
}

char *split_misc2 (char *argument, char *arg_first, char cEnd)
{
    while ( argument[0] < 32 && argument[0] != 0)
        argument++;
            
    while ( *argument != '\0' )
    {
        if ( *argument == cEnd)
        {
            argument++;
	    if (*argument != cEnd)
	        break;
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';
        
    return argument;
}


char *split_number_misc (char *argument, char *arg_first, char cEnd, int count)
{
    while ( isspace(*argument) )
        argument++;
            
    while ( *argument != '\0' )
    {
        if ( *argument == cEnd)
        {
	    if (--count < 1)
	    {
                argument++;
	    	if (*argument != cEnd)
	            break;
	    }
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';
        
    while ( isspace(*argument) )
        argument++;
            
    return argument;
}


/* is arg1 in arg 2 */
bool arg_check(char *arg1, char *arg2)
{
    char buf[MAX_INPUT_LENGTH];

     while (arg2[0] != '\0')
     {
	arg2 = one_argument(arg2, buf);
	if (!str_cmp(buf, arg1))
	    return TRUE;
     }

     return FALSE;
}

/* is any word from arg1 in arg2 */
bool multi_check(char *arg1, char *arg2)
{
    char buf[MAX_INPUT_LENGTH];
 
    while (arg1[0] != '\0')
    {
        arg1 = one_argument(arg1, buf);

        if (arg_check(buf, arg2))
	    return TRUE;
    }
    return FALSE;
}




/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;

    
    if (!IS_NPC(ch))
	return FALSE;

    if (!IS_NPC(victim))
    {
	if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
	    return TRUE;
	else
	    return FALSE;
    }

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->race == victim->race)
	return TRUE;
     
    if (IS_SET(ch->off_flags,ASSIST_ALIGN)
    &&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
    &&  ((IS_GOOD(ch) && IS_GOOD(victim))
    ||   (IS_EVIL(ch) && IS_EVIL(victim))
    ||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
	return TRUE;

    return FALSE;
}

void set_learnable_disciplines (CHAR_DATA *ch, int secrace)
{
    int i;
    
    crash("set learnable disciplines");

    /* this bit clears all the disciplines, just in case */
    for (i = 0 ; i < MAX_DISCIPLINES ; i++)
	ch->power[i] = -2;

    for (i = 0 ; i < 10 ; i++)
	ch->power[ ( (secrace-1) * 10 ) + i + 1] = -1;

    save_char_obj(ch);
}

void reset_learnable_disciplines (CHAR_DATA *ch, int secrace)
{
    int i;

    if (secrace < 1)
	return;

    crash("reset learnable disciplines");

    for (i = 0 ; i < 10 ; i++)
    {
	if (ch->power[( (secrace-1) * 10) + i + 1] < 0)
	    ch->power[ ( (secrace-1) * 10 ) + i + 1] = -1;
    }

    save_char_obj(ch);
}

void set_clan_disciplines(CHAR_DATA *ch)
{
    if (ch->power[clan_table[ch->clan].disc1] < 0)
        ch->power[clan_table[ch->clan].disc1] = 0;
    if (ch->power[clan_table[ch->clan].disc2] < 0)
        ch->power[clan_table[ch->clan].disc2] = 0;
    if (ch->power[clan_table[ch->clan].disc3] < 0)
        ch->power[clan_table[ch->clan].disc3] = 0;
}

/* scrolly text thang */
void scroll_to_char(char *text, CHAR_DATA *ch)
{
    char *ptr;

    ptr = text + strlen(text);

    while (ptr-- > text)
    {
	send_to_char(ptr, ch);
	send_to_char("\r",ch);
    }

    send_to_char("\n",ch);
}

bool can_regen_body_parts (CHAR_DATA *ch)
{
    if (ch->tertiaryrace == TERTIARY_LICH) return TRUE;

    return FALSE;
}

bool lose_body_part(CHAR_DATA *ch, int part)
{
    OBJ_DATA *obj = NULL;

    if (BODY_PART(ch, part))
        return FALSE;

    switch (part)
    {
	case BODY_RIGHT_LEG:
		if ((obj = get_eq_char(ch, WEAR_LEGS))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_FEET))!=NULL)
		    lose_obj(ch, obj);
		obj = create_body_part(ch, "right leg");
		SET_BIT(ch->body_form, part);
		SET_BIT(obj->extra_flags3, ITEM_BODY_LEG);

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your right leg grows back.\n\r", ch);
		    act("$n's right leg grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		return TRUE;
		break;
	case BODY_LEFT_LEG:
		if ((obj = get_eq_char(ch, WEAR_LEGS))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_FEET))!=NULL)
		    lose_obj(ch, obj);
		obj = create_body_part(ch, "left leg");
		SET_BIT(ch->body_form, part);
		SET_BIT(obj->extra_flags3, ITEM_BODY_LEG);

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your left leg grows back.\n\r", ch);
		    act("$n's left leg grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		return TRUE;
		break;
	case BODY_RIGHT_HAND:
		if ((obj = get_eq_char(ch, WEAR_FINGER_R))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_WRIST_R))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_RIGHT_HAND))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_HANDS))!=NULL)
		    lose_obj(ch, obj);
		obj = create_body_part(ch, "right hand");
		SET_BIT(ch->body_form, part);
		SET_BIT(obj->extra_flags3, ITEM_BODY_HAND);

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your right hand grows back.\n\r", ch);
		    act("$n's right hand grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		return TRUE;
		break;
	case BODY_LEFT_HAND:
		if ((obj = get_eq_char(ch, WEAR_FINGER_L))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_WRIST_L))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_LEFT_HAND))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_HANDS))!=NULL)
		    lose_obj(ch, obj);
		obj = create_body_part(ch, "left hand");
		SET_BIT(ch->body_form, part);
		SET_BIT(obj->extra_flags3, ITEM_BODY_HAND);

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your left hand grows back.\n\r", ch);
		    act("$n's left hand grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		return TRUE;
		break;
	case BODY_RIGHT_ARM:
		if ((obj = get_eq_char(ch, WEAR_FINGER_R))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_WRIST_R))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_RIGHT_HAND))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_HANDS))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_ARMS))!=NULL)
		    lose_obj(ch, obj);
		obj = create_body_part(ch, "right arm");
		SET_BIT(ch->body_form, part);
		SET_BIT(obj->extra_flags3, ITEM_BODY_ARM);
		SET_BIT(ch->body_form, BODY_RIGHT_HAND);

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your right arm grows back.\n\r", ch);
		    act("$n's right arm grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your right hand grows back.\n\r", ch);
		    act("$n's right hand grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		return TRUE;
		break;
	case BODY_LEFT_ARM:
		if ((obj = get_eq_char(ch, WEAR_FINGER_L))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_WRIST_L))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_LEFT_HAND))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_HANDS))!=NULL)
		    lose_obj(ch, obj);
		if ((obj = get_eq_char(ch, WEAR_ARMS))!=NULL)
		    lose_obj(ch, obj);
		obj = create_body_part(ch, "left arm");
		SET_BIT(ch->body_form, part);
		SET_BIT(ch->body_form, BODY_LEFT_HAND);
		SET_BIT(obj->extra_flags3, ITEM_BODY_ARM);

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your left arm grows back.\n\r", ch);
		    act("$n's left arm grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		if (can_regen_body_parts(ch))
	 	{
		    REMOVE_BIT(ch->body_form, part);
		    stc("Your left hand grows back.\n\r", ch);
		    act("$n's left hand grows back.", ch, NULL, NULL, TO_ROOM);		    
		}

		return TRUE;
		break;
	case BODY_THIRD_ARM:
		create_body_part(ch, "third arm");
		if ((obj = get_eq_char(ch, WEAR_THIRD_HAND))!=NULL)
		    lose_obj(ch, obj);
		REMOVE_BIT(ch->body_form, part);
		REMOVE_BIT(ch->body_form, BODY_THIRD_HAND);
		SET_TIMER(ch, TIMER_THIRD_ARM_GROWING, 0);
		SET_TIMER(ch, TIMER_THIRD_ARM_GOING, 0);
		return TRUE;
		break;
	case BODY_FOURTH_ARM:
		create_body_part(ch, "fourth arm");
		if ((obj = get_eq_char(ch, WEAR_FOURTH_HAND))!=NULL)
		    lose_obj(ch, obj);
		REMOVE_BIT(ch->body_form, part);
		REMOVE_BIT(ch->body_form, BODY_FOURTH_HAND);
		SET_TIMER(ch, TIMER_FOURTH_ARM_GROWING, 0);
		SET_TIMER(ch, TIMER_FOURTH_ARM_GOING, 0);
		return TRUE;
		break;
	case BODY_THIRD_HAND:
		create_body_part(ch, "third hand");
		if ((obj = get_eq_char(ch, WEAR_THIRD_HAND))!=NULL)
		    lose_obj(ch, obj);
		SET_BIT(ch->body_form, part);
		return TRUE;
		break;
	case BODY_FOURTH_HAND:
		create_body_part(ch, "fourth hand");
		if ((obj = get_eq_char(ch, WEAR_FOURTH_HAND))!=NULL)
		    lose_obj(ch, obj);
		SET_BIT(ch->body_form, part);
		return TRUE;
		break;
    }

    return FALSE;
}

void lose_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (ch == NULL || obj == NULL)
	return;

    obj_from_char(obj);
    obj_to_room(obj, ch->in_room, ch->altitude);

    act("$p clatters to the floor.", ch, obj, NULL, TO_ROOM);
    act("$p clatters to the floor.", ch, obj, NULL, TO_CHAR);
}

OBJ_DATA * create_body_part(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    obj = create_object( get_obj_index(14), 0 );

    sprintf(buf, "%s %s", GET_NAME(ch), argument);
    if (obj->name != NULL)
	free_string(obj->name);
    obj->name = str_dup(buf);
    sprintf(buf, "%s's %s", GET_NAME(ch), argument);
    if (obj->short_descr != NULL)
	free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);
    sprintf(buf, "%s's %s is lying here.", GET_NAME(ch), argument);
    if (obj->description != NULL)
	free_string(obj->description);
    obj->description = str_dup(buf);
    obj->item_type = ITEM_BODY_PART;
    WRITE_STR(obj->voodoo_name, GET_NAME(ch));

    if (!IS_NPC(ch)) SET_BIT(obj->extra_flags2, ITEM_VOO_BODY);

    obj_to_room(obj, ch->in_room, ch->altitude);

    return obj;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}
     
/* returns material number */
int material_lookup (const char *name)
{
    return 0;
}

/* returns race number */
int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_table[race].name != NULL; race++)
   {
	if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	&&  !str_prefix( name,race_table[race].name))
	    return race;
   }

   return 0;
} 

int liq_lookup (const char *name)
{
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
	    return liq;
    }

    return -1;
}

int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }
 
    return -1;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return weapon_table[type].type;
    }
 
    return WEAPON_EXOTIC;
}

int item_lookup(const char *name)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(item_table[type].name[0])
	&&  !str_prefix(name,item_table[type].name))
	    return item_table[type].type;
    }
 
    return -1;
}

char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "none";
}

char *weapon_name( int weapon_type)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
	if (weapon_type == weapon_table[type].type)
	    return weapon_table[type].name;
    return "exotic";
}

int attack_lookup  (const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
	    return att;
    }

    return 0;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    /* Firstly, do the backbone for each race, and then do the powers,
	which are not at all race specific - so that ie, eladrin can
	learn protean and be immune to fire! */

    /* Eladrin aura deals with itself, as it is switched on and off
	and removes and sets the bits in both */

    if (ch->secondaryrace == RACE_VAMPIRE)
    {
	/* They are resistant to cold */
	if (dam_type == DAM_COLD) immune = IS_RESISTANT;
 	/* They are vulnerable to drowning */
	if (dam_type == DAM_DROWNING) immune = IS_VULNERABLE;
	/* They are vulnerable to fire */
	if (dam_type == DAM_FIRE) immune = IS_VULNERABLE;
    }

    /* Firstly, we'll do flesh of lava! */
    if (ch->power[DISC_VAMP_PROT] >= 7
        && dam_type == DAM_FIRE) immune = IS_RESISTANT;

    /* True faith level 4 - Damning flame */
    if (dam_type == DAM_FIRE && ch->power[DISC_HERO_TRUE] >= 4
	&& ch->true_faith >= 5)
    {
	immune = IS_IMMUNE;
	stc("The flames lick around you, and you feel no heat.\n\r", ch);
    }

    /* The corruption discipline */
    if (dam_type == DAM_POISON && ch->power[DISC_DAEM_CORR] >= 3)
	immune = IS_IMMUNE;
    else if (dam_type == DAM_POISON && ch->power[DISC_DAEM_CORR] >= 1)
	immune = IS_RESISTANT;

    if (dam_type == DAM_DISEASE && ch->power[DISC_DAEM_CORR] >= 3)
	immune = IS_IMMUNE;
    else if (dam_type == DAM_DISEASE && ch->power[DISC_DAEM_CORR] >= 1)
	immune = IS_RESISTANT;

    /* The hellfire discipline */
    if (dam_type == DAM_FIRE && ch->power[DISC_DAEM_HELL] >= 3)
	immune = IS_IMMUNE;
    else if (dam_type == DAM_FIRE && ch->power[DISC_DAEM_HELL] >= 2)
	immune = IS_RESISTANT;

    /* The geluge discipline */
    if (dam_type == DAM_COLD && ch->power[DISC_DAEM_GELU] >= 4)
	immune = IS_IMMUNE;
    else if (dam_type == DAM_COLD && ch->power[DISC_DAEM_GELU] >= 1)
	immune = IS_RESISTANT;

    /* The cobra discipline */
    if (dam_type == DAM_POISON && ch->power[DISC_LUPI_COBR] >= 6)
	immune = IS_IMMUNE;
    else if (dam_type == DAM_POISON && ch->power[DISC_LUPI_COBR] >= 1)
	immune = IS_RESISTANT;

    /* Fortitude level 1 for vampires */
    if (dam_type == DAM_POISON && ch->power[DISC_VAMP_FORT] >= 1 && immune != IS_IMMUNE)
	immune = IS_RESISTANT;

    /* Carapace overrides */
    if (dam_type == DAM_BASH && has_psi(ch, "carapace"))
	immune = IS_RESISTANT;

    /* Total overrides */
    if (is_undead(ch) && dam_type == DAM_DISEASE) immune = IS_IMMUNE;

    if (ch->power[DISC_ELAD_MELO] >= 6 && dam_type == DAM_SOUND) immune = IS_RESISTANT;

    if (ch->secondaryrace == RACE_DAEMON
	&& dam_type == DAM_HOLY) immune = IS_VULNERABLE;

    if (can_breathe_underwater(ch)
	&& dam_type == DAM_DROWNING) immune = IS_IMMUNE;

    /* Returns immune if it has been changed at all ! */
    if (immune != -1) return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {   
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):         bit = IMM_BASH;         break;
	case(DAM_PIERCE):       bit = IMM_PIERCE;       break;
	case(DAM_SLASH):        bit = IMM_SLASH;        break;
	case(DAM_FIRE):         bit = IMM_FIRE;         break;
	case(DAM_COLD):         bit = IMM_COLD;         break;
	case(DAM_LIGHTNING):    bit = IMM_LIGHTNING;    break;
	case(DAM_ACID):         bit = IMM_ACID;         break;
	case(DAM_POISON):       bit = IMM_POISON;       break;
	case(DAM_NEGATIVE):     bit = IMM_NEGATIVE;     break;
	case(DAM_HOLY):         bit = IMM_HOLY;         break;
	case(DAM_ENERGY):       bit = IMM_ENERGY;       break;
	case(DAM_MENTAL):       bit = IMM_MENTAL;       break;
	case(DAM_DISEASE):      bit = IMM_DISEASE;      break;
	case(DAM_DROWNING):     bit = IMM_DROWNING;     break;
	case(DAM_LIGHT):        bit = IMM_LIGHT;        break;
	case(DAM_CHARM):        bit = IMM_CHARM;        break;
	case(DAM_SOUND):        bit = IMM_SOUND;        break;
	default:                return def;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
	return immune;
}

bool is_clan(CHAR_DATA *ch)
{
    return ch->clan;
}

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (clan_table[ch->clan].independent)
	return FALSE;
    else 
	return (ch->clan == victim->clan);
}

bool is_same_secondaryrace(CHAR_DATA *ch, CHAR_DATA *victim)
{
    return (ch->secondaryrace == victim->secondaryrace);
}

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}

int get_treelev(int sn, sh_int level)
{
    int i;
    char buf[200];

    if (level == 1)
    {
	if (skill_table[sn].spell_fun != spell_null)
	{
	    if (skill_table[sn].realm == REALM_WATER
		|| skill_table[sn].realm == REALM_LIFE)
	        return SKILL_PRIEST;
	    else return SKILL_WIZARD;
	}

	for (i = 0 ; i < MAX_SKILL_TREE ; i++)
	{
	    if (sn == *skill_tree[i].pgsn) return skill_tree[i].level1;
	}
	sprintf(buf, "Skill %s not in tree table - level 1.",
		skill_table[sn].name);
	bug (buf, 0);
	return SKILL_FUCKED;
    }

    if (level == 2)
    {
	if (skill_table[sn].spell_fun != spell_null)
	{
	    if (skill_table[sn].realm == REALM_WATER
		|| skill_table[sn].realm == REALM_LIFE)
	        return SKILL_PRIE_SPELLS;
	    else return SKILL_WIZA_SPELLS;
	}

	for (i = 0 ; i < MAX_SKILL_TREE ; i++)
	{
	    if (sn == *skill_tree[i].pgsn) return skill_tree[i].level2;
	}
	sprintf(buf, "Skill %s not in tree table - level 2.",
		skill_table[sn].name);
	bug (buf, 0);
	return SKILL_FUCKED;
    }
    bug ("Wrong level passed to get tree lev", 0);
    return SKILL_FUCKED;
}
 
/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    /* Changed by Jander for the skill tree */
    int skill = 10;

    crash("get skill - start");

    if (get_treelev(sn, 1) == SKILL_FUCKED) return 0; 

    /* For the success psionic power! */
    if (IS_FLAG5(ch, AFF5_SUCCESS))
    {
	REMOVE_BIT(ch->flag5, AFF5_SUCCESS);
	return 100;
    }

    if (ch->level == 120) return 100;

    crash("get skill place 2");

    if (skill_table[sn].spell_fun != spell_null && !IS_NPC(ch) )
    {
	skill = ch->pcdata->learned[sn]; 
	if (skill > 50) skill = 50;
	/* Adds the level 1 group - always the same */
	if (skill_table[sn].realm == REALM_WATER
	    || skill_table[sn].realm == REALM_LIFE)
	{
	    skill += UMIN(ch->pcdata->treelearn[SKILL_PRIEST], 20);
	    skill += UMIN(ch->pcdata->treelearn[SKILL_PRIE_SPELLS], 20);
	}
	else
	{
	    skill += UMIN(ch->pcdata->treelearn[SKILL_WIZARD], 20);
	    skill += UMIN(ch->pcdata->treelearn[SKILL_WIZA_SPELLS], 20);
	}
	if (skill_table[sn].realm == ch->primary)
		skill = UMIN(100, skill + 30);
    }

    else if (!IS_NPC(ch))
    {
	skill = ch->pcdata->learned[sn];

	/* All skills are limited to 50 for the time being
	    they can go above, but this gives a possible 100+
	    for the actual skill - bad, maybe i can expand this in
	    the future */

	if (skill > 50) skill = 50; 
	skill += UMIN(ch->pcdata->treelearn[get_treelev(sn, 1)], 20);
	skill += UMIN(ch->pcdata->treelearn[get_treelev(sn, 2)], 20);
    }

    else /* mobiles */
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill = 40 + 2 * ch->level;

	else if (sn == gsn_sneak || sn == gsn_hide)
	    skill = ch->level * 2 + 20;

	else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
	    skill = ch->level * 2 + 40;

	else if (sn == gsn_shield_block)
	    skill = 30 + 2 * ch->level;

	else if (sn == gsn_hand_to_hand)
	    skill = 60 + 2 * ch->level;

	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 40 + 3 * ch->level;

	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 40 + 3 * ch->level;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->act,ACT_WARRIOR)
	     ||   IS_SET(ch->act,ACT_THIEF)))
	    skill = 30 + 3 * ch->level;

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * ch->level;

	else if (sn == gsn_kick)
	    skill = 50 + 3 * ch->level;

	else if (sn == gsn_backstab && IS_SET(ch->act,ACT_THIEF))
	    skill = 20 + 2 * ch->level;

	else if (sn == gsn_rescue)
	    skill = 40 + ch->level; 

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm)
	    skill = 40 + 5 * ch->level / 2;

	else 
	   skill = 0;
    }

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    crash("get skill place 3");

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    if (skill < 20) check_improve(ch, sn, TRUE, 1);
    else if (skill < 50 && dice(1, 2) == 1) check_improve(ch, sn, TRUE, 1);
    crash("well its reached the end of get_skill");
    if (IS_FLAG5(ch, AFF5_MISFORTUNE)) skill = dice(1, skill);
    return URANGE(10,skill,100);
}

bool is_dagger(OBJ_DATA *obj)
{
    if (obj == NULL) return FALSE;
    if (obj->item_type != ITEM_WEAPON) return FALSE;

    if (is_name("dagger", obj->name)) return TRUE;
    if (is_name("knife", obj->name)) return TRUE;
    if (is_name("spear", obj->name)) return TRUE;
    if (is_name("dart", obj->name)) return TRUE;
    if (is_name("javelin", obj->name)) return TRUE;

    return FALSE;
}

OBJ_DATA * get_dagger_char(CHAR_DATA *ch)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (is_dagger(held)) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (is_dagger(held)) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (is_dagger(held)) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (is_dagger(held)) return held;
    return FALSE;
}

/* search hands for object type */

bool is_obj_held(CHAR_DATA *ch, sh_int iType)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && held->item_type == iType) return TRUE;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && held->item_type == iType) return TRUE;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && held->item_type == iType) return TRUE;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && held->item_type == iType) return TRUE;
    return FALSE;
}

OBJ_DATA *get_obj_by_name(CHAR_DATA *ch, char *name)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && !str_cmp(name, held->short_descr)) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && !str_cmp(name, held->short_descr)) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && !str_cmp(name, held->short_descr)) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && !str_cmp(name, held->short_descr)) return held;
    return NULL;
}

OBJ_DATA *get_obj_by_type(CHAR_DATA *ch, sh_int iType)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && held->item_type == iType) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && held->item_type == iType) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && held->item_type == iType) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && held->item_type == iType) return held;
    return NULL;
}

OBJ_DATA *get_obj_by_location(CHAR_DATA *ch, int wf)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && IS_SET(held->wear_flags, wf)) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && IS_SET(held->wear_flags, wf)) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && IS_SET(held->wear_flags, wf)) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && IS_SET(held->wear_flags, wf)) return held;
    return NULL;
}

OBJ_DATA *get_obj_by_stat2(CHAR_DATA *ch, sh_int iType)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && IS_OBJ_STAT2(held, iType) ) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && IS_OBJ_STAT2(held, iType) ) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && IS_OBJ_STAT2(held, iType) ) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && IS_OBJ_STAT2(held, iType) ) return held;
    return NULL;
}

OBJ_DATA *get_obj_by_stat(CHAR_DATA *ch, sh_int iType)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && IS_OBJ_STAT(held, iType) ) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && IS_OBJ_STAT(held, iType) ) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && IS_OBJ_STAT(held, iType) ) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && IS_OBJ_STAT(held, iType) ) return held;
    return NULL;
}

OBJ_DATA *get_obj_by_ntype(CHAR_DATA *ch, sh_int iType, int cnt)
{
    OBJ_DATA *held;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && held->item_type == iType) cnt--;
    if (cnt == 0) return held;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && held->item_type == iType) cnt--;
    if (cnt == 0) return held;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && held->item_type == iType) cnt--;
    if (cnt == 0) return held;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && held->item_type == iType) cnt--;
    if (cnt == 0) return held;
    return NULL;
}

int count_obj_by_type(CHAR_DATA *ch, sh_int iType)
{
    OBJ_DATA *held;
    int cnt = 0;

    held = get_eq_char(ch, WEAR_RIGHT_HAND);
    if (held != NULL && held->item_type == iType) cnt++;
    held = get_eq_char(ch, WEAR_LEFT_HAND);
    if (held != NULL && held->item_type == iType) cnt++;
    held = get_eq_char(ch, WEAR_THIRD_HAND);
    if (held != NULL && held->item_type == iType) cnt++;
    held = get_eq_char(ch, WEAR_FOURTH_HAND);
    if (held != NULL && held->item_type == iType) cnt++;
    return cnt;
}


/* for returning weapon information */
int get_weapon_sn_by_type(OBJ_DATA *wield)
{
    int sn;

    if (wield == NULL || wield->item_type != ITEM_WEAPON)
	sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
	default :               sn = -1;                break;
	case(WEAPON_SWORD):     sn = gsn_sword;         break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
	case(WEAPON_SPEAR):     sn = gsn_spear;         break;
	case(WEAPON_MACE):      sn = gsn_mace;          break;
	case(WEAPON_AXE):       sn = gsn_axe;           break;
	case(WEAPON_FLAIL):     sn = gsn_flail;         break;
	case(WEAPON_WHIP):      sn = gsn_whip;          break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
   }
   return sn;
}


int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_obj_by_type( ch, ITEM_WEAPON );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
	sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
	default :               sn = -1;                break;
	case(WEAPON_SWORD):     sn = gsn_sword;         break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
	case(WEAPON_SPEAR):     sn = gsn_spear;         break;
	case(WEAPON_MACE):      sn = gsn_mace;          break;
	case(WEAPON_AXE):       sn = gsn_axe;           break;
	case(WEAPON_FLAIL):     sn = gsn_flail;         break;
	case(WEAPON_WHIP):      sn = gsn_whip;          break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
   }
   return sn;
}

int get_weapon_snsl(CHAR_DATA *ch, int sl)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, sl );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
	sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
	default :               sn = -1;                break;
	case(WEAPON_SWORD):     sn = gsn_sword;         break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
	case(WEAPON_SPEAR):     sn = gsn_spear;         break;
	case(WEAPON_MACE):      sn = gsn_mace;          break;
	case(WEAPON_AXE):       sn = gsn_axe;           break;
	case(WEAPON_FLAIL):     sn = gsn_flail;         break;
	case(WEAPON_WHIP):      sn = gsn_whip;          break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
} 

/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc,mod,stat;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;

     if (IS_NPC(ch))
	return;

    if (ch->pcdata->perm_hit == 0 
    ||  ch->pcdata->perm_move == 0
    ||  ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:     ch->sex         -= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_HIT:     ch->max_hit     -= mod;         break;
		    case APPLY_MOVE:    ch->max_move    -= mod;         break;
		}
	    }

	    for ( af = obj->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:     ch->sex         -= mod;         break;
		    case APPLY_HIT:     ch->max_hit     -= mod;         break;
		    case APPLY_MOVE:    ch->max_move    -= mod;         break;
		}
	    }
	}
	/* now reset the permanent stats */
	ch->pcdata->perm_hit    = ch->max_hit;
	ch->pcdata->perm_move   = ch->max_move;
	ch->pcdata->last_level  = ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		if (ch->sex > 0 && ch->sex < 3)
		    ch->pcdata->true_sex        = ch->sex;
		else
		    ch->pcdata->true_sex        = 0;

    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0; 
    ch->sex             = ch->pcdata->true_sex;
    ch->max_hit         = ch->pcdata->perm_hit;
    ch->max_move        = ch->pcdata->perm_move;
   
    for (i = 0; i < 4; i++)
	ch->armor[i]    = 100;

    ch->hitroll         = 0;
    ch->damroll         = 0;
    ch->saving_throw    = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
	obj = get_eq_char(ch,loc);
	if (obj == NULL)
	    continue;
	for (i = 0; i < 4; i++)
	    ch->armor[i] -= apply_ac( obj, loc, i );

	if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	{
	    mod = af->modifier;
	    switch(af->location)
	    {
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
		case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;

		case APPLY_SEX:         ch->sex                 += mod; break;
		case APPLY_HIT:         ch->max_hit             += mod; break;
		case APPLY_MOVE:        ch->max_move            += mod; break;

		case APPLY_AC:          
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
		case APPLY_DAMROLL:     ch->damroll             += mod; break;
	
		case APPLY_SAVES:               ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	    }
	}
 
	for ( af = obj->affected; af != NULL; af = af->next )
	{
	    mod = af->modifier;
	    switch(af->location)
	    {
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
		case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
 
		case APPLY_SEX:         ch->sex                 += mod; break;
		case APPLY_HIT:         ch->max_hit             += mod; break;
		case APPLY_MOVE:        ch->max_move            += mod; break;
 
		case APPLY_AC:
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
		case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
		case APPLY_SAVES:         ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	    }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
	mod = af->modifier;
	switch(af->location)
	{
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
		case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
 
		case APPLY_SEX:         ch->sex                 += mod; break;
		case APPLY_HIT:         ch->max_hit             += mod; break;
		case APPLY_MOVE:        ch->max_move            += mod; break;
 
		case APPLY_AC:
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
		case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
		case APPLY_SAVES:         ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	} 
    }

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if (ch->trust <= 120 && ch->trust > 0)
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    int addage = 0;

    if (!IS_NPC(ch)) addage = ch->pcdata->extra_age;

    return 17 + addage + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;
    int bonus = 0;

    crash("get_curr_stat");

    if (IS_NPC(ch)) return 12;  // It doesnt really matter does it?!?

    /* Put all the racial modifiers in here, so losing discipline
	levels will actually remove the thingys. */

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	max = 30;

    if (ch->level == 120)
	return 30;
    else
    {
	max = pc_race_table[ch->race].max_stats[stat] + 4;

	if ( ch->race == race_lookup("human"))
	    max += 1;

	max = UMIN(max,30);
    }

    if (stat == STAT_STR)
    {
	bonus += UMAX(ch->power[DISC_VAMP_POTE], 0);
	bonus += UMAX(ch->power[DISC_LUPI_URSI], 0);
	bonus += UMAX(ch->power[DISC_DAEM_ATTA], 0);
	bonus += UMAX(ch->power[DISC_ELAD_PHYS] * 3 / 2, 0);
	bonus += UMAX(ch->power[DISC_DRAG_MIGT], 0);
	if (ch->secondaryrace == RACE_DRAGON) bonus += ch->generation;
    }

    if (stat == STAT_DEX)
    {
	bonus += UMAX(ch->power[DISC_VAMP_CELE], 0);
	bonus += UMAX(ch->power[DISC_LUPI_LUPI], 0);
	bonus += UMAX(ch->power[DISC_DAEM_ATTA] / 2, 0);
	bonus += UMAX(ch->power[DISC_ELAD_PHYS] * 3 / 2,0);
	bonus += UMAX(ch->power[DISC_DRAG_META], 0);
        bonus += UMAX(ch->power[DISC_HERO_BATT] / 2, 0);
    }

    if (stat == STAT_CON)
    {
	bonus += UMAX(ch->power[DISC_VAMP_FORT], 0);
	bonus += UMAX(ch->power[DISC_LUPI_BOAR], 0);
        bonus += UMAX(ch->power[DISC_DAEM_IMUN], 0);
	if (ch->secondaryrace == RACE_DRAGON) bonus += ch->generation;
    }

    if (stat == STAT_CHA)
    {
	bonus += UMAX(ch->power[DISC_VAMP_PRES], 0);
	bonus += UMAX(ch->power[DISC_DRAG_AWE ], 0);
    }

  
    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat] + bonus, max);
}

	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    crash("can_carry_n");

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if (ch->cur_form != 0 && form_data[ch->cur_form].max_items != -1)
	return ((form_data[ch->cur_form].can_wear) ? MAX_WEAR : 0 ) + form_data[ch->cur_form].max_items;

    return MAX_WEAR + get_curr_stat(ch,STAT_DEX) / 3;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    crash("can_carry_w");

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if (ch->cur_form != 0 && form_data[ch->cur_form].carry_max != -1)
	return form_data[ch->cur_form].carry_max * 10;

    return str_app[get_curr_stat(ch,STAT_STR)].carry * 20;
}



/*
 * See if a string is one of the names of an object.
 */

bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    /* fix crash on NULL namelist */
    if (namelist == NULL || namelist[0] == '\0')
	return FALSE;

    /* fixed to prevent is_name on "" returning TRUE */
    if (str[0] == '\0')
	return FALSE;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

bool is_exact_name(char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if (namelist == NULL)
	return FALSE;

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *paf, *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected;
	     paf != NULL; paf = paf->next)
	{
	    af_new = new_affect();

	    af_new->next = obj->affected;
	    obj->affected = af_new;
 
	    af_new->where       = paf->where;
	    af_new->type        = UMAX(0,paf->type);
	    af_new->level       = paf->level;
	    af_new->duration    = paf->duration;
	    af_new->location    = paf->location;
	    af_new->modifier    = paf->modifier;
	    af_new->bitvector   = paf->bitvector;
	}
    }
}
	   

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	case TO_NEWFLAGS:
	    SET_BIT(ch->flag2, paf->bitvector);
	    break;
	case TO_FLAG3:
	    SET_BIT(ch->flag3, paf->bitvector);
	    break;
	case TO_FLAG4:
	    SET_BIT(ch->flag4, paf->bitvector);
	    break;
	case TO_FLAG5:
	    SET_BIT(ch->flag5, paf->bitvector);
	    break;
	case TO_FLAG6:
	    SET_BIT(ch->flag6, paf->bitvector);
	    break;
	}
    }
    else
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
	    REMOVE_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    REMOVE_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    REMOVE_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    REMOVE_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	case TO_NEWFLAGS:
	    REMOVE_BIT(ch->flag2,paf->bitvector);
	    break;
	case TO_FLAG3:
	    REMOVE_BIT(ch->flag3,paf->bitvector);
	    break;
	case TO_FLAG4:
	    REMOVE_BIT(ch->flag4,paf->bitvector);
	    break;
	case TO_FLAG5:
	    REMOVE_BIT(ch->flag5,paf->bitvector);
	    break;
	case TO_FLAG6:
	    REMOVE_BIT(ch->flag6,paf->bitvector);
	    break;
	}
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:

	if (paf->location != 12) bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:                                            break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]    += mod; break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]    += mod; break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]    += mod; break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]    += mod; break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]    += mod; break;
    case APPLY_CHA:           ch->mod_stat[STAT_CHA]    += mod; break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_LEVEL:                                           break;
    case APPLY_AGE:                                             break;
    case APPLY_HEIGHT:                                          break;
    case APPLY_WEIGHT:                                          break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:                                            break;
    case APPLY_EXP:                                             break;
    case APPLY_AC:
	for (i = 0; i < 4; i ++)
	    ch->armor[i] += mod;
	break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVES:   ch->saving_throw                += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    case APPLY_SPELL_AFFECT:                                    break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && ( wield = get_obj_by_type( ch, ITEM_WEAPON ) ) != NULL
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10)
    &&   IS_FLAG4(ch, AFF_QUITTING))
    
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room, ch->altitude );
	    depth--;
	}
    }

    return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
	if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
		case TO_AFFECTS:
		    SET_BIT(ch->affected_by,vector);
		    break;
		case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
		case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
		case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
		case TO_NEWFLAGS:
		    SET_BIT(ch->flag2,vector);
		    break;
		case TO_FLAG3:
		    SET_BIT(ch->flag3,vector);
		    break;
		case TO_FLAG4:
		    SET_BIT(ch->flag4,vector);
		    break;
		case TO_FLAG5:
		    SET_BIT(ch->flag5,vector);
		    break;
		case TO_FLAG6:
		    SET_BIT(ch->flag6,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

	    for (paf = obj->affected; paf != NULL; paf = paf->next)
	    if (paf->where == where && paf->bitvector == vector)
	    {
		switch (where)
		{
		    case TO_AFFECTS:
			SET_BIT(ch->affected_by,vector);
			break;
		    case TO_IMMUNE:
			SET_BIT(ch->imm_flags,vector);
			break;
		    case TO_RESIST:
			SET_BIT(ch->res_flags,vector);
			break;
		    case TO_VULN:
			SET_BIT(ch->vuln_flags,vector);
			break;                  
		    case TO_NEWFLAGS:
			SET_BIT(ch->flag2,vector);
			break;                  
		    case TO_FLAG3:
			SET_BIT(ch->flag3,vector);
			break;                  
		    case TO_FLAG4:
			SET_BIT(ch->flag4,vector);
			break;                  
		    case TO_FLAG5:
			SET_BIT(ch->flag5,vector);
			break;                  
		    case TO_FLAG6:
			SET_BIT(ch->flag6,vector);
			break;                  
		}
		return;
	    }

	if (obj->enchanted)
	    continue;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	    if (paf->where == where && paf->bitvector == vector)
	    {
		switch (where)
		{
		    case TO_AFFECTS:
			SET_BIT(ch->affected_by,vector);
			break;
		    case TO_IMMUNE:
			SET_BIT(ch->imm_flags,vector);
			break;
		    case TO_RESIST:
			SET_BIT(ch->res_flags,vector);
			break;
		    case TO_VULN:
			SET_BIT(ch->vuln_flags,vector);
			break;
		    case TO_NEWFLAGS:
			SET_BIT(ch->flag2,vector);
			break;
		    case TO_FLAG3:
			SET_BIT(ch->flag3,vector);
			break;
		    case TO_FLAG4:
			SET_BIT(ch->flag4,vector);
			break;
		    case TO_FLAG5:
			SET_BIT(ch->flag5,vector);
			break;
		    case TO_FLAG6:
			SET_BIT(ch->flag6,vector);
			break;
		}
		return;
	    }
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new            = *paf;

    paf_new->name = NULL; 
    paf_new->end_message = NULL;

    paf_new->next       = ch->affected;
    ch->affected        = paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/*
 * Give an affect to a char  - give it a name.
 */
void affect_name_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new            = *paf;

    paf_new->next       = ch->affected;
    ch->affected        = paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new            = *paf;
    paf_new->next       = obj->affected;
    obj->affected       = paf_new;

    paf_new->name = NULL;
    paf_new->end_message = NULL;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
	switch (paf->where)
	{
	case TO_OBJECT:
	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
	case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
		SET_BIT(obj->value[4],paf->bitvector);
	    break;
	}
    

    return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;

    if ( ch->affected == NULL )
    {
	bug( "Affect_remove: no affect.", 0 );
	return;
    }

    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( paf == ch->affected )
    {
	ch->affected    = paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
    if ( obj->affected == NULL )
    {
	bug( "Affect_remove_object: no affect.", 0 );
	return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
	{
	case TO_OBJECT:
	    REMOVE_BIT(obj->extra_flags,paf->bitvector);
	    break;
	case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
		REMOVE_BIT(obj->value[4],paf->bitvector);
	    break;
	}

    if ( paf == obj->affected )
    {
	obj->affected    = paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = obj->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove_object: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect, but set the name as well?!?!?!
 *  I dont know how
 */
void affect_name_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_name_to_char( ch, paf );
}

/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->level = (paf->level += paf_old->level) / 2;

	    /* Bug fix for joining perm spells */
	    if (paf->duration != -1) paf->duration += paf_old->duration;

	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    paf->name = NULL;
    paf->end_message = NULL;
    affect_to_char( ch, paf );
    return;
}



/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    crash("char_from_room");

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }
 
    SS(1);

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    crash("char_from_room - pos 2");

    SS(2);

    if ( ( obj = get_obj_by_type( ch, ITEM_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    SS(3);

    if ( (obj = get_eq_char( ch, WEAR_LEFT_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW)
	&& ch->in_room->light > 0)
	--ch->in_room->light;

    crash("char_from_room - pos 2 and a bit");

    SS(4);

    if ( (obj = get_eq_char( ch, WEAR_RIGHT_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW)
	&& ch->in_room->light > 0)
	--ch->in_room->light;

    SS(5);

    if ( (obj = get_eq_char( ch, WEAR_THIRD_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW)
	&& ch->in_room->light > 0)
	--ch->in_room->light;

    SS(6);

    if ( (obj = get_eq_char( ch, WEAR_FOURTH_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW)
	&& ch->in_room->light > 0)
	--ch->in_room->light;

    SS(7);

    if (ch->riding != NULL && IS_NPC(ch->riding))
	char_from_room(ch->riding);

    crash("char_from_room - pos 2 and a bit and another bit");

    SS(8);

    if (!IS_NPC(ch) && ch->riden_by != NULL)
	char_from_room(ch->riden_by);

    SS(9);

    crash("char_from_room - ack ack ack");

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    SS(10);

    crash("char_from_room - jakalwere");

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->zone	     = NULL;
    ch->on           = NULL;  /* sanity check! */

    SS(11);

    return;
}


/************************
* REALM LOCATION        *
************************/

REALM_DATA * realm_by_number( int rnum )
{
    REALM_DATA *realm;

    for (realm = realm_list; realm != NULL; realm = realm->next)
	if (realm->vnum == rnum) return realm;

    return NULL;
}


REALM_DATA * get_realm( char *argument )
{
    REALM_DATA *realm;

    if (argument == NULL || argument[0] == 0)
	return NULL;

    crash("get_realm - ziggy");

    for (realm = realm_list; realm != NULL; realm = realm->next)
    {
	if (!str_cmp(realm->name, argument))
	    return realm;
    }
    crash("get_realm - stardust");

    return NULL;
}

REALM_DATA * char_realm( CHAR_DATA *ch )
{
    crash( "char_realm - nipples" );

    if (ch->realm != NULL)
	return ch->realm;

    crash( "char_realm - twisted" );

    if (ch->in_room != NULL)
    {
	ch->realm = ch->in_room->area->realm;
	return ch->realm;
    }

    crash( "char_realm - very" );

    if (get_realm("the prime material plane") != NULL)
    {
	ch->realm = get_realm("the prime material plane");
	return ch->realm;
    }

    crash( "char_realm - hard" );

    return NULL;
}

/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex, int alt )
{
    OBJ_DATA *obj;
    char buf[100];

    crash("char_to_room");

    if ( pRoomIndex == NULL )
    {
	ROOM_INDEX_DATA *room;

	sprintf(buf, "Char_to_room: %s going to NULL.", GET_NAME(ch) );
	bug(buf, 0);

	if ((room = get_room_index(prime_realm, 3054)) != NULL)
	    char_to_room(ch,room, 0);
	
	return;
    }

    ch->in_room         = pRoomIndex;
    ch->next_in_room    = pRoomIndex->people;
    pRoomIndex->people  = ch;
    ch->zone		= pRoomIndex->area;
    ch->realm		= ch->zone->realm;
    ch->altitude	= alt;

    REMOVE_BIT(ch->flag4, AFF_INVULNERABLE);

    /* Ok, this is the obj switching bit... confusing */
    sprintf(buf, "%s is just on his way to a room.", 
	GET_NAME(ch));
    crash(buf);
    check_char_sw_obj(ch);

    if (ch->sw_obj != NULL) move_obj_to_char(ch);

    crash("char_to_room yeah yeah yeah");

    /* Check if any quests have been completed */
    if (!IS_NPC(ch) && IS_FLAG4(ch, AFF_ON_A_QUEST))
    {
	if (ch->pcdata->quest_room_1 != NULL
	    && !str_cmp(ch->in_room->name, ch->pcdata->quest_room_1) )
	{
	    act("A #2green#n glow spreads outwards from your body.", ch, NULL, NULL, TO_CHAR);
	    act("A #2green#n glow spreads outwards from $n's body.", ch, NULL, NULL, TO_ROOM);
	    ch->pcdata->quest_room_1 = NULL;
	    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ROOM_1);
	}
	else if (ch->pcdata->quest_room_2 != NULL
	    && !str_cmp(ch->in_room->name, ch->pcdata->quest_room_2) )
	{
	    act("A #4blue#n glow spreads outwards from your body.", ch, NULL, NULL, TO_CHAR);
	    act("A #4blue#n glow spreads outwards from $n's body.", ch, NULL, NULL, TO_ROOM);
	    ch->pcdata->quest_room_2 = NULL;
	    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ROOM_2);
	}
	else if (ch->pcdata->quest_room_3 != NULL
	    && !str_cmp(ch->in_room->name, ch->pcdata->quest_room_3) )
	{
	    act("A #1red#n glow spreads outwards from your body.", ch, NULL, NULL, TO_CHAR);
	    act("A #1red#n glow spreads outwards from $n's body.", ch, NULL, NULL, TO_ROOM);
	    ch->pcdata->quest_room_3 = NULL;
	    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ROOM_3);
	}
    }

    crash("char_to_room play dat guitar");

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if (!IS_NPC(ch) && ch->zone->repop_status == REPOP_NEEDED) 
	reset_area( ch->zone );

    crash("char_to_room its all gone pair shaped");

    if ( ( obj = get_obj_by_type( ch, ITEM_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;
	
    if ( (obj = get_eq_char( ch, WEAR_LEFT_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW))
	++ch->in_room->light;

    if ( (obj = get_eq_char( ch, WEAR_RIGHT_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW))
	++ch->in_room->light;

    if ( (obj = get_eq_char( ch, WEAR_THIRD_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW))
	++ch->in_room->light;

    if ( (obj = get_eq_char( ch, WEAR_FOURTH_HAND ) ) != NULL
	&& IS_SET(obj->extra_flags, ITEM_GLOW))
	++ch->in_room->light;

    if (ch->riding != NULL && IS_NPC(ch->riding))
	char_to_room(ch->riding, pRoomIndex, ch->altitude);

    if (!IS_NPC(ch) && ch->riden_by != NULL)
	char_to_room(ch->riden_by, pRoomIndex, ch->altitude);

    crash("char_to_room my arms are wonky");

    REMOVE_BIT(ch->flag4, AFF_ENTWINED);
    REMOVE_BIT(ch->flag4, AFF_SMOTHERED);

    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
	AFFECT_DATA *af, plague;
	CHAR_DATA *vch;
	
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
	
	plague.where            = TO_AFFECTS;
	plague.type             = gsn_plague;
	plague.level            = af->level - 1; 
	plague.duration         = number_range(1,2 * plague.level);
	plague.location         = APPLY_STR;
	plague.modifier         = -3 - (ch->entropy / 3);
	plague.bitvector        = AFF_PLAGUE;
	
	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
	    &&  !IS_IMMORTAL(vch) &&
		!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
	    {
		send_to_char("You feel hot and feverish.\n\r",vch);
		act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
		affect_join(vch,&plague);
	    }
	}
    }

    crash("char_to_rom?");

    if (ch->shadow_master != NULL)
    {
	char_to_room(ch->shadow_master, pRoomIndex, ch->altitude);
	do_look(ch->shadow_master, "auto");
    }

    crash("char_to_room - end routine");
    return;
}

/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    char buf[1000];

    if (obj == NULL)
	crash("null obj in obj_to_char");

    else if (ch == NULL)
	crash("null ch in obj_to_char");

    else 
    {
	sprintf(buf, "obj_to_char : '%s' given to '%s'.",
	    obj->short_descr, ch->short_descr);
        crash(buf);
    }

    obj->next_content    = ch->carrying;
    ch->carrying         = obj;
    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
    obj->altitude	 = ch->altitude;

    if (obj->sw_char != NULL)
	move_char_to_obj(obj->sw_char);
}


/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    crash("obj_from_char");

    if ( obj == NULL )
    {
	crash("What??!?!!? A null object.  Must inform the cops.");
	return;
    }

    if ( ( ch = obj->carried_by ) == NULL )
    {
 	crash("Yes... its a null char!  Balls!");
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    REMOVE_BIT(obj->extra_flags2, ITEM_ITEMHIDE);

    obj->carried_by      = NULL;
    obj->next_content    = NULL;
    ch->carry_number    -= get_obj_number( obj );
    ch->carry_weight    -= get_obj_weight( obj );
    crash("obj_from_char - end routine");
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:     return 3 * obj->value[type];
    case WEAR_HEAD:     return 2 * obj->value[type];
    case WEAR_LEGS:     return 2 * obj->value[type];
    case WEAR_FEET:     return     obj->value[type];
    case WEAR_HANDS:    return     obj->value[type];
    case WEAR_ARMS:     return     obj->value[type];
    case WEAR_NECK_1:   return     obj->value[type];
    case WEAR_NECK_2:   return     obj->value[type];
    case WEAR_ABOUT:    return 2 * obj->value[type];
    case WEAR_WAIST:    return     obj->value[type];
    case WEAR_WRIST_L:  return     obj->value[type];
    case WEAR_WRIST_R:  return     obj->value[type];
    case WEAR_LEFT_HAND: return     obj->value[type];
    case WEAR_RIGHT_HAND: return     obj->value[type];
    case WEAR_THIRD_HAND:  return     obj->value[type];
    case WEAR_FOURTH_HAND: return     obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}


OBJ_DATA *get_random_armour( CHAR_DATA *ch )
{
    int iWear, count = 0, randitem;

    crash("get random armour");

    if (ch == NULL)
	return NULL;

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if (get_eq_char( ch, iWear) != NULL)
	    count += 1;
    }

    if (count < 1)
	return NULL;

    randitem = dice(1, count);

    crash("get random armour place 2");

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if (get_eq_char( ch, iWear) != NULL)
	    randitem -= 1;
	if (randitem < 1)
	    return get_eq_char( ch, iWear );
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    int i;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    if ( ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) 
    ) && ch->level < 120 )
    {
	act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room, ch->altitude );
	return;
    }

    for (i = 0; i < 4; i++)
	ch->armor[i]            -= apply_ac( obj, iWear,i );
    obj->wear_loc        = iWear;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location != APPLY_SPELL_AFFECT )
		affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
	    affect_to_char ( ch, paf );
	else
	    affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    if ( IS_SET(obj->extra_flags, ITEM_GLOW))
	++ch->in_room->light;

    /* Perm spells ..... remember to update strip_perm_spell in heart.c */
    /*  Always use affect join... just in case */

    if (obj->perm_spell != -1) /* That is... there is a perm spell on it */
    {
	AFFECT_DATA af;

	switch( obj->perm_spell )
 	{
	    case ITEMA_SANCTUARY:
	  	if (is_affected(ch, gsn_sanctuary)) return;
                af.where     = TO_AFFECTS;
                af.type      = skill_lookup("sanctuary");  
                af.level     = obj->level;
                af.duration  = -1;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_SANCTUARY;
                affect_join( ch, &af );
	 	act("A white radiance spreads outwards from $p and surrounds you.", ch, obj, NULL, TO_CHAR);
	 	act("A white radiance spreads outwards from $p and surrounds $n.", ch, obj, NULL, TO_ROOM);
	        break;
	  
	    case ITEMA_HASTE:
	  	if (is_affected(ch, skill_lookup("haste"))) return;
	        af.where     = TO_AFFECTS;
                af.type      = skill_lookup("haste");
       	        af.level     = obj->level;
       	        af.duration  = -1;
       		af.location  = APPLY_DEX;
                af.modifier  = 1 + (obj->level >= 18) + (obj->level >= 25) +
                           (obj->level >= 32);
                af.bitvector = AFF_HASTE; 
                affect_join( ch, &af );
		act("$p starts humming slightly.  Your reactions speed up.", ch, obj, NULL, TO_CHAR);
		act("$p starts humming slightly, $n starts to move faster.", ch, obj, NULL, TO_ROOM);
		break;

	    case ITEMA_WINDTOMB:
	  	if (is_affected(ch, skill_lookup("windtomb"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("windtomb");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = -40 - (ch->air * 2);
    		af.location  = APPLY_AC;
    		af.bitvector = 0;
    		affect_join( ch, &af );
    		send_to_char( "You are enveloped within a roaring hurricane.\n\r", ch );
    		act("A roaring wind envelops $n.",ch,NULL,NULL,TO_CHAR);
		break;

	    case ITEMA_FLY:
	  	if (is_affected(ch, skill_lookup("fly"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("fly");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0;
    		af.location  = 0;
    		af.bitvector = AFF_FLYING;
	        send_to_char( "Your feet rise off the ground.\n\r", ch );
   		act( "$n's feet rise off the ground.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_ACCURACY:
	  	if (is_affected(ch, skill_lookup("accuracy"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("accuracy");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = ch->air * 4;
    		af.location  = APPLY_HITROLL;
    		af.bitvector = 0;
	        send_to_char( "Your now have an improved awareness of combat.\n\r", ch );
   		act( "$p sparkles slightly.", ch, obj, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_DETECT_EVIL:
	  	if (is_affected(ch, skill_lookup("detect evil"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("detect evil");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0;
    		af.location  = APPLY_NONE;
    		af.bitvector = AFF_DETECT_EVIL;
	        send_to_char( "Your eyes tingle.\n\r", ch );
   		act( "$p glows softly.", ch, obj, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_INVISIBILITY:
	  	if (is_affected(ch, skill_lookup("invisibility"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("invisibility");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0;
    		af.location  = APPLY_NONE;
    		af.bitvector = AFF_INVISIBLE;
	        send_to_char( "You fade out of existence.\n\r", ch );
   		act( "$n fades out of existence.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_DETECT_MAGIC:
	  	if (is_affected(ch, skill_lookup("detect magic"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("detect magic");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0;
    		af.location  = APPLY_NONE;
    		af.bitvector = AFF_DETECT_MAGIC;
	        send_to_char( "Your eyes tingle slightly.\n\r", ch );
    		affect_join( ch, &af );
		break;

	    case ITEMA_ARMOR:
	  	if (is_affected(ch, skill_lookup("armour"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("armour");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0 - (ch->earth * 4);
		if (af.modifier > -20) af.modifier = -20;
    		af.location  = APPLY_AC;
    		af.bitvector = 0;
	        send_to_char( "Your armour is enhanced by a cantrip.\n\r", ch );
   		act( "$n's armour glows softly as it is enhanced by a cantrip.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_STONE_SKIN:
	  	if (is_affected(ch, skill_lookup("stone skin"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("stone skin");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = -30;
    		af.location  = APPLY_AC;
    		af.bitvector = 0;
	        send_to_char( "Your skin hardens and turns to stone.\n\r", ch );
   		act( "$n's skin hardens and turns to stone.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_SHIELD:
	  	if (is_affected(ch, skill_lookup("shield"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("shield");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = -20;
    		af.location  = APPLY_AC;
    		af.bitvector = 0;
	        send_to_char( "You are surrounded by a shield of energy.\n\r", ch );
   		act( "$n is surrounded by a shield of energy.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_DETECT_GOOD:
	  	if (is_affected(ch, skill_lookup("detect good"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("detect good");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = -20;
    		af.location  = APPLY_NONE;
    		af.bitvector = AFF_DETECT_GOOD;
	        send_to_char( "Your eyes tingle.\n\r", ch );
    		affect_join( ch, &af );
		break;

	    case ITEMA_BLESS:
	  	if (is_affected(ch, skill_lookup("bless"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("bless");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = ch->level / (11 - ch->water);;
    		af.location  = APPLY_HITROLL;
    		af.bitvector = 0;
	        send_to_char( "You feel favoured by the gods.\n\r", ch );
    		affect_join( ch, &af );
		break;

	    case ITEMA_WATER_BREATHING:
	  	if (is_affected(ch, skill_lookup("water breathing"))) return;
	        af.where     = TO_NEWFLAGS;
    	   	af.type      = skill_lookup("water breathing");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0;
    		af.location  = 0;
    		af.bitvector = AFF_WATERBREATHING;
	        send_to_char( "Your neck starts to itch as small gills break the surface.\n\r", ch );
   		act( "Small gills appear on $n's neck.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_IRON_SKIN:
	  	if (is_affected(ch, skill_lookup("iron skin"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("iron skin");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = -40;
    		af.location  = APPLY_AC;
    		af.bitvector = 0;
	        send_to_char( "Your skin is now as hard as iron.\n\r", ch );
   		act( "$n's skin turns to iron.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_PLAGUE:
	  	if (is_affected(ch, skill_lookup("plague"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("plague");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = 0 - (ch->entropy / 2) - 2;
    		af.location  = APPLY_STR;
    		af.bitvector = AFF_PLAGUE;
	        send_to_char( "You scream in agony as plague sores erupt through your skin.\n\r", ch );
   		act( "$n's screams in agony as plague sores erupt through $s skin.", ch, NULL, NULL, TO_ROOM );
    		affect_join( ch, &af );
		break;

	    case ITEMA_PROTECTION_EVIL:
	  	if (is_affected(ch, skill_lookup("protection evil"))) return;
	        af.where     = TO_AFFECTS;
    	   	af.type      = skill_lookup("protection evil");
  		af.level     = obj->level;
   		af.duration  = -1;
    		af.modifier  = -1;
    		af.location  = APPLY_SAVING_SPELL;
    		af.bitvector = AFF_PROTECT_EVIL;
	        send_to_char( "You feel pure.\n\r", ch );
    		affect_join( ch, &af );
		break;

	    default : break;
	}
    }
    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
	ch->armor[i]    += apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc        = -1;

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location == APPLY_SPELL_AFFECT )
	    {
		for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
		{
		    lpaf_next = lpaf->next;
		    if ((lpaf->type == paf->type) &&
			(lpaf->level == paf->level) &&
			(lpaf->location == APPLY_SPELL_AFFECT))
		    {
			affect_remove( ch, lpaf );
			lpaf_next = NULL;
		    }
		}
	    }
	    else
	    {
		affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
	{
	    bug ( "Norm-Apply: %d", 0 );
	    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	    {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		{
		    bug ( "location = %d", lpaf->location );
		    bug ( "type = %d", lpaf->type );
		    affect_remove( ch, lpaf );
		    lpaf_next = NULL;
		}
	    }
	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector); 
	}

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( IS_SET(obj->extra_flags, ITEM_GLOW))
	--ch->in_room->light;

    if (obj->perm_spell != -1) strip_perm_spell(ch, obj);

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex, int alt )
{
    if (obj == NULL) return;
    if (pRoomIndex == NULL)
    {
	obj_to_room(obj, get_room_index(prime_realm, 47), 0);
	return;
    }

    obj->next_content           = pRoomIndex->contents;
    pRoomIndex->contents        = obj;
    obj->in_room                = pRoomIndex;
    obj->carried_by             = NULL;
    obj->in_obj                 = NULL;

    if (IS_OBJ_STAT2(obj, ITEM_ICE))
    {
	extract_obj(obj);
	obj = create_object(get_obj_index(95), 0);
	obj->timer = 3;
	obj_to_room(obj, pRoomIndex, obj->altitude);
    }

    obj->altitude = alt;

    /* And the oswitching bit */
    if (obj->sw_char != NULL)
	move_char_to_obj(obj->sw_char);

    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content           = obj_to->contains;
    obj_to->contains            = obj;
    obj->in_obj                 = obj_to;
    obj->in_room                = NULL;
    obj->carried_by             = NULL;

    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
	obj->cost = 0; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    /* And the oswitching bit */
    if (obj->sw_char != NULL)
	move_char_to_obj(obj->sw_char);

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *mob, *ch;
    CHAR_DATA *victim;

    crash("oooh, naughty extract_obj has crashed it!");

    // Obvious fix that was not in the original code
    if (obj == NULL) return;
    
    /* Deal with infinite looping on char return */
    REMOVE_BIT(obj->extra_flags3, ITEM_DESTROY_CHAR_RETURN);

    /* check if any nasty elementals escape */

    if (obj->animated != NULL)
    {
	act("$n collapses into a pool of black liquid which seeps into the floor.", obj->animated, NULL, NULL, TO_ROOM);
	consume_kill( obj->animated, FALSE ); /* NO BLOOD */
	obj->animated = NULL;
    }

    crash("extract obj II");

    if (IS_WEAPON_STAT(obj, WEAPON_ELE_FLAME) && locate_obj(obj) != NULL
	&& obj->item_type == ITEM_WEAPON)
    {
	sprintf(buf, "A fire elemental bursts forth from %s.", obj->short_descr);
	room_message(locate_obj(obj), buf );
	victim = ((locate_obj(obj))->people);
	mob = create_mobile(get_mob_index(1) );
	char_to_room(mob, locate_obj(obj), obj->altitude);
	mob->altitude = obj->altitude;
	if (victim != NULL) set_fighting (mob, victim);
    }
    if (IS_WEAPON_STAT(obj, WEAPON_ELE_WATER) && locate_obj(obj) != NULL
	&& obj->item_type == ITEM_WEAPON)
    {
	sprintf(buf, "A water elemental bursts forth from %s.", obj->short_descr);
	room_message(locate_obj(obj), buf );
	victim = ((locate_obj(obj))->people);
	mob = create_mobile(get_mob_index(2) );
	char_to_room(mob, locate_obj(obj), obj->altitude);
	mob->altitude = obj->altitude;
	if (victim != NULL) set_fighting (mob, victim);
    }
    if (IS_WEAPON_STAT(obj, WEAPON_ELE_AIR) && locate_obj(obj) != NULL
	&& obj->item_type == ITEM_WEAPON)
    {
	sprintf(buf, "An air elemental bursts forth from %s.", obj->short_descr);
	room_message(locate_obj(obj), buf );
	victim = ((locate_obj(obj))->people);
	mob = create_mobile(get_mob_index(3) );
	char_to_room(mob, locate_obj(obj), obj->altitude);
	mob->altitude = obj->altitude;
	if (victim != NULL) set_fighting (mob, victim);
    }
    if (IS_WEAPON_STAT(obj, WEAPON_ELE_EARTH) && locate_obj(obj) != NULL
	&&  obj->item_type == ITEM_WEAPON)
    {
	sprintf(buf, "An earth elemental bursts forth from %s.", obj->short_descr);
	room_message(locate_obj(obj), buf );
	victim = ((locate_obj(obj))->people);
	mob = create_mobile(get_mob_index(4) );
	char_to_room(mob, locate_obj(obj), obj->altitude);
	mob->altitude = obj->altitude;
	if (victim != NULL) set_fighting (mob, victim);
    }

    if (obj->item_type == ITEM_CONT_LIGHT && obj->in_room != NULL)
	REMOVE_BIT(obj->in_room->room_flags, ROOM_CONT_LIGHT);

    crash("extract obj III");

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    /* Get heads when they are sacced */
    if ( (ch = obj->sw_char) != NULL
	&& IS_SET(obj->extra_flags3, ITEM_DESTROY_CHAR_IF_SACCED))
    {
	int i;

	char_from_obj(ch);
	make_blood(ch);
	ch->hit = 1;
	ch->move = 1;
	ch->psp = 1;
        REMOVE_BIT(ch->flag4, AFF_ENTWINED);
        REMOVE_BIT(ch->flag4, AFF_SMOTHERED);
	char_from_room(ch);
	char_to_room(ch, get_room_index(prime_realm, ROOM_VNUM_START), 0);
	ch->currentexp = 0;
	while(ch->affected)
	    affect_remove(ch, ch->affected);
	ch->pcdata->condition[COND_HUNGER] = 60;
	ch->pcdata->condition[COND_THIRST] = 60;
	for (i = 0 ; i < 4 ; i++)
	    ch->armor[i] = 100;
  	save_char_obj(ch);
        ch = NULL; /* SaNiTy? */
    }

    /* Knacker people within! */
    else if ((ch = obj->sw_char) != NULL)
    {
	char_from_obj(ch);
	act("You are wrenched from $p as it is destroyed.", ch, obj, NULL, TO_CHAR);
	act("$n is wrenched from $p as it is destroyed.", ch, obj, NULL, TO_ROOM);
	ch->hit = -10;
	update_pos(ch);
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }
    crash("extract obj end");

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}


/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    close_progs(ch);

    if ( ch->in_room == NULL )
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }
    nuke_pets(ch);
    ch->pet = NULL; /* just in case */


    if ( fPull )
	die_follower( ch );
    
    stop_fighting( ch, TRUE );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
	extract_obj( obj );
    }
    
    char_from_room( ch );

    if (ch->riding != NULL)
	ch->riding->riden_by = NULL;

    if (ch->riden_by != NULL)
	ch->riden_by->riding = NULL;

    /* Death room is set in the clan tabe now */
    if ( !fPull )
    {
	char_to_room(ch,get_room_index(prime_realm, clan_table[ch->clan].hall), 0);
	return;
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
    {
	do_return( ch, "" );
	ch->desc = NULL;
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
    }

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}
    }

    if ( ch->desc != NULL )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;
    bool gotform = FALSE;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if (rch->altitude != ch->altitude) continue;
	if (rch->cur_form != 0
	    && !str_prefix(arg, form_data[rch->cur_form].lookname) )
	    gotform = TRUE;

	if (can_see_name(ch, rch))
	{
	    if (!str_prefix(arg, GET_PC_NAME(rch)))
		gotform = TRUE;
	}
	
	if ( ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	&& !(rch->cur_form != 0 && gotform) )
	    continue;

	if ( ++count == number )
	    return rch;
    }

    return NULL;
}
/*
* gets a char from a ROOM that you say
*/

CHAR_DATA *get_char_exit( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char*argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;
    bool gotform = FALSE;

    number = number_argument( argument, arg );
    if ( !str_cmp( arg, "self" ) )
	return ch;
    count  = 0;
    for ( rch = room->people; rch != NULL; rch = rch->next_in_room )
    {
	if (rch->altitude != ch->altitude) continue;
	if (rch->cur_form != 0
	    && !str_prefix(arg, form_data[rch->cur_form].lookname) )
	    gotform = TRUE;
	
	if (can_see_name(ch, rch))
	{
	    if (!str_prefix(arg, GET_PC_NAME(rch)))
		gotform = TRUE;
	}
	
	if ( ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	&& !(rch->cur_form != 0 && gotform) )
	    continue;

	if ( ++count == number )
	    return rch;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;
    bool gotform = FALSE;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if (wch->cur_form != 0
	    && !str_prefix(arg, form_data[wch->cur_form].lookname) )
	    gotform = TRUE;
	
	if (can_see_name(ch, wch))
	{
	    if (!str_prefix(arg, GET_PC_NAME(wch)))
		gotform = TRUE;
	}
	
	if (( wch->in_room == NULL || !can_see( ch, wch ) 
	||   !is_name( arg, wch->name ) )
	&& !(wch->cur_form != 0 && gotform) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   (can_see_obj( viewer, obj ) ) 
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

OBJ_DATA *get_obj_eq( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( (can_see_obj( viewer, obj ) ) 
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
    int silver = 0, gold = 0;

    silver = UMIN(ch->silver,cost); 

    if (silver < cost)
    {
	gold = ((cost - silver + 99) / 100);
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    if (ch->gold < 0)
    {
	bug("deduct costs: gold %d < 0",ch->gold);
	ch->gold = 0;
    }
    if (ch->silver < 0)
    {
	bug("deduct costs: silver %d < 0",ch->silver);
	ch->silver = 0;
    }
}   

bool enough_money(CHAR_DATA *ch, long amount)  // in copper
{
    if (TOTAL_WORTH(ch) >= amount) return TRUE;
    return FALSE;
}

void give_cash(CHAR_DATA *ch, CHAR_DATA *victim, long amount)   // in copper
{
    int gold = 0, silver = 0, copper = 0, emeralds = 0, sapphires = 0, diamonds = 0;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char comma[2];

    comma[0] = 0;
    comma[1] = 0;

    if (amount >= CNV_DIA)
    {
	diamonds = UMIN(ch->diamonds, amount / CNV_DIA);
	ch->diamonds -= diamonds;
	amount -= diamonds * CNV_DIA;
    }

    if (amount >= CNV_SAP)
    {
	sapphires = UMIN(ch->sapphires, amount / CNV_SAP);
	ch->sapphires -= sapphires;
	amount -= sapphires * CNV_SAP;
    }

    if (amount >= CNV_EME)
    {
	emeralds = UMIN(ch->emeralds, amount / CNV_EME);
	ch->emeralds -= emeralds;
	amount -= emeralds * CNV_EME;
    }

    if (amount >= CNV_GOL)
    {
	gold = UMIN(ch->gold, amount / CNV_GOL);
	ch->gold -= gold;
	amount -= gold * CNV_GOL;
    }

    if (amount >= CNV_SIL)
    {
	silver = UMIN(ch->silver, amount / CNV_SIL);
	ch->silver -= silver;
	amount -= silver * CNV_SIL;
    }

    if (amount >= CNV_COP)
    {
	copper = UMIN(ch->copper, amount);
	ch->copper -= copper;
	amount -= copper;
    }

    if (victim != NULL)
    {
    	victim->copper += copper;
    	victim->silver += silver;
    	victim->gold += gold;
    	victim->emeralds += emeralds;
    	victim->sapphires += sapphires;
    	victim->diamonds += diamonds;
    }

    buf[0] = 0;
    if (diamonds > 0)
    {
	sprintf(buf2, "%s %d diamonds", comma, diamonds);
	strcat(buf, buf2);
	comma[0] = ',';
    }
    if (sapphires > 0)
    {
	sprintf(buf2, "%s %d sapphires", comma, sapphires);
	strcat(buf, buf2);
	comma[0] = ',';
    }
    if (emeralds > 0)
    {
	sprintf(buf2, "%s %d emeralds", comma, emeralds);
	strcat(buf, buf2);
	comma[0] = ',';
    }
    if (gold > 0)
    {
	sprintf(buf2, "%s %d gold", comma, gold);
	strcat(buf, buf2);
	comma[0] = ',';
    }
    if (silver > 0)
    {
	sprintf(buf2, "%s %d silver", comma, silver);
	strcat(buf, buf2);
	comma[0] = ',';
    }
    if (copper > 0)
    {
	sprintf(buf2, "%s %d copper", comma, copper);
	strcat(buf, buf2);
	comma[0] = ',';
    }
    if (comma[0] != ',')
	strcat(buf, "nothing");
    if (victim == NULL)
    {
	sprintf(buf2, "You are now %s poorer.", buf);
	stc(buf2, ch);
	return;
    }
    sprintf(buf2, "You give $N %s.", buf);
    act(buf2, ch, NULL, victim, TO_CHAR);
    sprintf(buf2, "$n gives you %s.", buf);
    act(buf2, ch, NULL, victim, TO_VICT);
    sprintf(buf2, "$n gives $N %s.", buf);
    act(buf2, ch, NULL, victim, TO_NOTVICT);
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",UMIN(gold,silver));
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
	sprintf( buf, obj->short_descr, gold );
	free_string( obj->short_descr );
	obj->short_descr        = str_dup( buf );
	obj->value[1]           = gold;
	obj->cost               = gold;
	obj->weight             = gold/5;
    }
    else if (gold == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
	sprintf( buf, obj->short_descr, silver );
	free_string( obj->short_descr );
	obj->short_descr        = str_dup( buf );
	obj->value[0]           = silver;
	obj->cost               = silver;
	obj->weight             = silver/20;
    }
 
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
	sprintf( buf, obj->short_descr, silver, gold );
	free_string( obj->short_descr );
	obj->short_descr        = str_dup( buf );
	obj->value[0]           = silver;
	obj->value[1]           = gold;
	obj->cost               = 100 * gold + silver;
	obj->weight             = gold / 5 + silver / 20;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY
    || IS_OBJ_STAT2(obj, ITEM_FORM_MELDED))
	number = 0;
    else
	number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
	number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    if (IS_OBJ_STAT2(obj, ITEM_FORM_MELDED))
	return 0;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    if (IS_OBJ_STAT2(obj, ITEM_FORM_MELDED))
	return 0;

    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
	weight += get_obj_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if (!RTIME_UP(pRoomIndex, RTIMER_DARK_ROOM)) return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_CONT_LIGHT) )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;


    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
	return TRUE;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
    {
	if (rch->altitude != 0) continue;
	count++;
    }

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    /* Hiding rooms with nasty spells */

    if (IS_IMMORTAL(ch)) return TRUE;

    if (!RTIME_UP(pRoomIndex, RTIMER_HIDE_ROOM)
	&& ch->power[DISC_LUPI_HAWK] < 8
	&& ch->power[DISC_VAMP_AUSP] < 8
	&& ch->alignment < 200) return FALSE;
	
    return TRUE;
}


bool can_see_invis(CHAR_DATA *ch)
{
    if (IS_IMPLEMENTOR(ch)) return TRUE;
    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if (IS_AFFECTED(ch, AFF_DETECT_INVIS) ) return TRUE;

    if (ch->power[DISC_LUPI_HAWK] >= 8 ) return TRUE; /* Hawk sight */

    if (ch->power[DISC_VAMP_AUSP] >= 4 ) return TRUE; /* Vamp truesight */

    if (ch->power[DISC_LUPI_LUNA] >= 6
	&& IS_NIGHT() ) return TRUE; /* Luna invis */

    return FALSE;
}

bool can_see_hidden(CHAR_DATA *ch)
{
    if (IS_IMPLEMENTOR(ch)) return TRUE;
    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;  

    if (IS_AFFECTED(ch,AFF_DETECT_HIDDEN)) return TRUE; /* Spell */

    if (ch->power[DISC_LUPI_HAWK] >= 8 ) return TRUE; /* Hawk sight */

    if (ch->power[DISC_VAMP_AUSP] >= 3 ) return TRUE; /* Dark sense */

    if (ch->power[DISC_LUPI_LUNA] >= 5
	&& IS_NIGHT() ) return TRUE; /* Luna hidden */

    if (ch->pet != NULL && IS_FLAG5(ch->pet, AFF5_FAMILIAR_HAWK) ) return TRUE;

    return FALSE;
}

bool can_see_shadows(CHAR_DATA *ch)
{
    if (IS_IMPLEMENTOR(ch)) return TRUE;

    if (IS_NEWFLAG(ch, AFF_SHADOW_PLANE)) return TRUE;

    if (IS_NEWFLAG(ch, AFF_SHADOW_SIGHT)) return TRUE;

    /* Warlocks with mist eyes can see the shadow plane */
    if (ch->power[DISC_WARL_THEA] >= 5) return TRUE;

    return FALSE;
}

bool can_see_in_dark(CHAR_DATA *ch)
{
    if (IS_IMPLEMENTOR(ch)) return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( IS_AFFECTED(ch, AFF_INFRARED) ) return TRUE;

    if ( IS_AFFECTED(ch, AFF_DARK_VISION) ) return TRUE;
 
    if (ch->secondaryrace == RACE_LUPINE) return TRUE;

    if (ch->power[DISC_LUPI_LUNA] >= 1) return TRUE; /* Lunas guidence */

    if (ch->power[DISC_LUPI_HAWK] >= 8) return TRUE; /* hawk sight */

    if (ch->power[DISC_VAMP_PROT] >= 1) return TRUE; /* gleam of red eyes */

    if (ch->power[DISC_DAEM_HELL] >= 1) return TRUE; /* Inner flame */

    if (ch->power[DISC_HERO_ELEM] >= 4) return TRUE; /* Gift of light */

    if (ch->pet != NULL && IS_FLAG5(ch->pet, AFF5_FAMILIAR_CAT) ) return TRUE;

    return FALSE;
}


/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch == victim )
	return TRUE;

    /* People within containers cannot see anybody else! */
    if (ch->sw_obj != NULL && ch->sw_obj->in_obj != NULL)
	return FALSE;

    if (is_item(victim)) return FALSE;

    if (victim->blinkykill != NULL) return FALSE;
   
    if (IS_CREATOR(ch)) return TRUE;

    if (IS_SET(victim->flag4, AFF_SHADOW_HIDE)) return FALSE;
    
    if (IS_SET(victim->flag4, AFF_SENESCHIA)) return FALSE;

    if (ch->in_room != NULL
	&& !RTIME_UP(ch->in_room, RTIMER_STINKING_CLOUD) )
	return FALSE;

    if (IS_FLAG4(victim, AFF_FLICKER) && dice(1,2) == 1) return FALSE;

    if ( get_trust(ch) < victim->invis_level)
	return FALSE;

    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_NEWFLAG(ch, AFF_EARTH_MELD) )
	return FALSE;

    if ( IS_FLAG4(ch, AFF_TREE_MELD) )
	return FALSE;

    if ( IS_NEWFLAG(victim, AFF_EARTH_MELD) )
	return FALSE;

    if ( IS_FLAG4(victim, AFF_TREE_MELD) )
	return FALSE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !can_see_in_dark(ch) )
	return FALSE;

    if ( (IS_AFFECTED(victim, AFF_INVISIBLE)
    ||   IS_NEWFLAG(victim, AFF_IMPROVED_INVIS))
    && !can_see_invis(ch) )
	return FALSE;

    if (IS_NEWFLAG(victim, AFF_SHADOW_PLANE)
	&& !can_see_shadows(ch) )
	return FALSE;

    if (IS_NEWFLAG(ch, AFF_SHADOW_PLANE)
	&& !can_see_shadows(victim) )
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !can_see_hidden(ch)
    &&   victim->fighting == NULL)
    {
	int chance;
	chance = get_skill(victim,gsn_sneak);
	chance += get_curr_stat(victim,STAT_DEX) * 3/2;
	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance -= ch->level - victim->level * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !can_see_hidden(ch)
    &&   victim->fighting == NULL)
	return FALSE;

    return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if (IS_OBJ_STAT2(obj, ITEM_FORM_MELDED))
	return FALSE;

    /* Big mama oswitching check */
    if (ch->sw_obj != NULL && ch->sw_obj == obj) return FALSE;
   
    /* The debugged altitude check... you can see items that
	you are carrying or people in the same room are carrying */

    if (obj_owner(obj) != NULL
	&& (obj_owner(obj))->altitude != ch->altitude) return FALSE;

    if (obj_owner(obj) == NULL
	&& obj->altitude != ch->altitude) return FALSE;

    if (IS_CREATOR(ch)) return TRUE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if (IS_OBJ_STAT2(obj, ITEM_ITEMHIDE)
	&& !can_see_invis(ch)) return FALSE;

    if ( IS_NEWFLAG(ch, AFF_EARTH_MELD) )
	return FALSE;

    if ( IS_FLAG4(ch, AFF_TREE_MELD) )
	return FALSE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !can_see_invis(ch) )
	return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !can_see_in_dark(ch) )
	return FALSE;

    return TRUE;
}

/* Can see object, but no altitude checks */
bool can_see_obj_no_alt( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if (IS_OBJ_STAT2(obj, ITEM_FORM_MELDED))
	return FALSE;
   
    if (IS_CREATOR(ch)) return TRUE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if (IS_OBJ_STAT2(obj, ITEM_ITEMHIDE)
	&& !can_see_in_dark(ch) ) return FALSE;

    if ( IS_NEWFLAG(ch, AFF_EARTH_MELD) )
	return FALSE;

    if ( IS_FLAG4(ch, AFF_TREE_MELD) )
	return FALSE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !can_see_invis(ch) )
	return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !can_see_in_dark(ch) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if (IS_SET(obj->extra_flags2, ITEM_DESIRED)) return FALSE;

    if ( !IS_SET(obj->extra_flags, ITEM_NODROP))
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    /* Demons are unaffected by cursed items */
    if ( IS_SET(obj->extra_flags, ITEM_NODROP)
	  && ch->power[DISC_DAEM_CORR] >= 2) return TRUE;

    return FALSE;
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:            return "none";
    case APPLY_STR:             return "strength";
    case APPLY_DEX:             return "agility";
    case APPLY_INT:             return "intelligence";
    case APPLY_WIS:             return "wisdom";
    case APPLY_CON:             return "resilience";
    case APPLY_CHA:             return "charisma";
    case APPLY_SEX:             return "sex";
    case APPLY_LEVEL:           return "level";
    case APPLY_AGE:             return "age";
    case APPLY_HIT:             return "hp";
    case APPLY_MOVE:            return "moves";
    case APPLY_GOLD:            return "gold";
    case APPLY_EXP:             return "experience";
    case APPLY_AC:              return "armor class";
    case APPLY_HITROLL:         return "hit roll";
    case APPLY_DAMROLL:         return "damage roll";
    case APPLY_SAVES:           return "saves";
    case APPLY_SAVING_ROD:      return "save vs rod";
    case APPLY_SAVING_PETRI:    return "save vs petrification";
    case APPLY_SAVING_BREATH:   return "save vs breath";
    case APPLY_SAVING_SPELL:    return "save vs spell";
    case APPLY_SPELL_AFFECT:    return "none";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " detect_good"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_PROTECT_EVIL  ) strcat( buf, " prot_evil"     );
    if ( vector & AFF_PROTECT_GOOD  ) strcat( buf, " prot_good"     );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_BERSERK       ) strcat( buf, " berserk"       );
    if ( vector & AFF_CALM          ) strcat( buf, " calm"          );
    if ( vector & AFF_HASTE         ) strcat( buf, " haste"         );
    if ( vector & AFF_SLOW          ) strcat( buf, " slow"          );
    if ( vector & AFF_PLAGUE        ) strcat( buf, " plague"        );
    if ( vector & AFF_DARK_VISION   ) strcat( buf, " dark_vision"   );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_ARTIFACT     ) strcat( buf, " artifact"     );
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti-good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti-evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_NOPURGE      ) strcat( buf, " nopurge"      );
    if ( extra_flags & ITEM_VIS_DEATH    ) strcat( buf, " vis_death"    );
    if ( extra_flags & ITEM_ROT_DEATH    ) strcat( buf, " rot_death"    );
    if ( extra_flags & ITEM_NOLOCATE     ) strcat( buf, " no_locate"    );
    if ( extra_flags & ITEM_SELL_EXTRACT ) strcat( buf, " sell_extract" );
    if ( extra_flags & ITEM_BURN_PROOF   ) strcat( buf, " burn_proof"   );
    if ( extra_flags & ITEM_NOUNCURSE    ) strcat( buf, " no_uncurse"   );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    { 
	strcat(buf," npc");
	if (act_flags & ACT_SENTINEL    ) strcat(buf, " sentinel");
	if (act_flags & ACT_SCAVENGER   ) strcat(buf, " scavenger");
	if (act_flags & ACT_AGGRESSIVE  ) strcat(buf, " aggressive");
	if (act_flags & ACT_STAY_AREA   ) strcat(buf, " stay_area");
	if (act_flags & ACT_WIMPY       ) strcat(buf, " wimpy");
	if (act_flags & ACT_PET         ) strcat(buf, " pet");
	if (act_flags & ACT_TRAIN       ) strcat(buf, " train");
	if (act_flags & ACT_PRACTICE    ) strcat(buf, " practice");
	if (act_flags & ACT_UNDEAD      ) strcat(buf, " undead");
	if (act_flags & ACT_CLERIC      ) strcat(buf, " cleric");
	if (act_flags & ACT_MAGE        ) strcat(buf, " mage");
	if (act_flags & ACT_THIEF       ) strcat(buf, " thief");
	if (act_flags & ACT_WARRIOR     ) strcat(buf, " warrior");
	if (act_flags & ACT_NOALIGN     ) strcat(buf, " no_align");
	if (act_flags & ACT_NOPURGE     ) strcat(buf, " no_purge");
	if (act_flags & ACT_IS_HEALER   ) strcat(buf, " healer");
	if (act_flags & ACT_IS_CHANGER  ) strcat(buf, " changer");
	if (act_flags & ACT_GAIN        ) strcat(buf, " skill_train");
	if (act_flags & ACT_UPDATE_ALWAYS) strcat(buf," update_always");
	if (act_flags & ACT_RIDEABLE	) strcat(buf," rideable");
    }
    else
    {
	strcat(buf," player");
	if (act_flags & PLR_AUTOASSIST  ) strcat(buf, " autoassist");
	if (act_flags & PLR_AUTOEXIT    ) strcat(buf, " autoexit");
	if (act_flags & PLR_AUTOLOOT    ) strcat(buf, " autoloot");
	if (act_flags & PLR_AUTOSAC     ) strcat(buf, " autosac");
	if (act_flags & PLR_AUTOGOLD    ) strcat(buf, " autogold");
	if (act_flags & PLR_AUTOSPLIT   ) strcat(buf, " autosplit");
	if (act_flags & PLR_HOLYLIGHT   ) strcat(buf, " holy_light");
	if (act_flags & PLR_CANLOOT     ) strcat(buf, " loot_corpse");
	if (act_flags & PLR_NOSUMMON    ) strcat(buf, " no_summon");
	if (act_flags & PLR_NOFOLLOW    ) strcat(buf, " no_follow");
	if (act_flags & PLR_FREEZE      ) strcat(buf, " frozen");
	if (act_flags & PLR_THIEF       ) strcat(buf, " thief");
	if (act_flags & PLR_KILLER      ) strcat(buf, " killer");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET         ) strcat(buf, " quiet");
    if (comm_flags & COMM_DEAF          ) strcat(buf, " deaf");
    if (comm_flags & COMM_NOWIZ         ) strcat(buf, " no_wiz");
    if (comm_flags & COMM_NOAUCTION     ) strcat(buf, " no_auction");
    if (comm_flags & COMM_NOGOSSIP      ) strcat(buf, " no_gossip");
    if (comm_flags & COMM_NOQUESTION    ) strcat(buf, " no_question");
    if (comm_flags & COMM_NOMUSIC       ) strcat(buf, " no_music");
    if (comm_flags & COMM_NOQUOTE       ) strcat(buf, " no_quote");
    if (comm_flags & COMM_COMPACT       ) strcat(buf, " compact");
    if (comm_flags & COMM_BRIEF         ) strcat(buf, " brief");
    if (comm_flags & COMM_PROMPT        ) strcat(buf, " prompt");
    if (comm_flags & COMM_COMBINE       ) strcat(buf, " combine");
    if (comm_flags & COMM_NOEMOTE       ) strcat(buf, " no_emote");
    if (comm_flags & COMM_NOSHOUT       ) strcat(buf, " no_shout");
    if (comm_flags & COMM_NOTELL        ) strcat(buf, " no_tell");
    if (comm_flags & COMM_NOCHANNELS    ) strcat(buf, " no_channels");
    

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON          ) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM           ) strcat(buf, " charm");
    if (imm_flags & IMM_MAGIC           ) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON          ) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH            ) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE          ) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH           ) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE            ) strcat(buf, " fire");
    if (imm_flags & IMM_COLD            ) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING       ) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID            ) strcat(buf, " acid");
    if (imm_flags & IMM_POISON          ) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE        ) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY            ) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY          ) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL          ) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE ) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING        ) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT           ) strcat(buf, " light");
    if (imm_flags & VULN_IRON           ) strcat(buf, " iron");
    if (imm_flags & VULN_WOOD           ) strcat(buf, " wood");
    if (imm_flags & VULN_SILVER ) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE          ) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER   ) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK     ) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY     ) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD     ) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS     ) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET     ) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS    ) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS     ) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_ABOUT    ) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST    ) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST    ) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD         ) strcat(buf, " wield");
    if (wear_flags & ITEM_WEAR_SHIELD   ) strcat(buf, " shield");
    if (wear_flags & ITEM_HOLD          ) strcat(buf, " hold");
    if (wear_flags & ITEM_NO_SAC        ) strcat(buf, " nosac");
    if (wear_flags & ITEM_WEAR_FLOAT    ) strcat(buf, " float");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON        ) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE   ) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL       ) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY ) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER         ) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL        ) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT      ) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD        ) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT     ) strcat(buf, " construct");
    if (form_flags & FORM_MIST          ) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE    ) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED         ) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR       ) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT        ) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER        ) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN    ) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM          ) strcat(buf, " worm");
    if (form_flags & FORM_BLOB          ) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL        ) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD          ) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE       ) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE         ) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON        ) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN     ) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH          ) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD    ) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD          ) strcat(buf, " head");
    if (part_flags & PART_ARMS          ) strcat(buf, " arms");
    if (part_flags & PART_LEGS          ) strcat(buf, " legs");
    if (part_flags & PART_HEART         ) strcat(buf, " heart");
    if (part_flags & PART_BRAINS        ) strcat(buf, " brains");
    if (part_flags & PART_GUTS          ) strcat(buf, " guts");
    if (part_flags & PART_HANDS         ) strcat(buf, " hands");
    if (part_flags & PART_FEET          ) strcat(buf, " feet");
    if (part_flags & PART_FINGERS       ) strcat(buf, " fingers");
    if (part_flags & PART_EAR           ) strcat(buf, " ears");
    if (part_flags & PART_EYE           ) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE   ) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS     ) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES     ) strcat(buf, " tentacles");
    if (part_flags & PART_FINS          ) strcat(buf, " fins");
    if (part_flags & PART_WINGS         ) strcat(buf, " wings");
    if (part_flags & PART_TAIL          ) strcat(buf, " tail");
    if (part_flags & PART_CLAWS         ) strcat(buf, " claws");
    if (part_flags & PART_FANGS         ) strcat(buf, " fangs");
    if (part_flags & PART_HORNS         ) strcat(buf, " horns");
    if (part_flags & PART_SCALES        ) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING   ) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST     ) strcat(buf, " frost");
    if (weapon_flags & WEAPON_VAMPIRIC  ) strcat(buf, " vampiric");
    if (weapon_flags & WEAPON_SHARP     ) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_VORPAL    ) strcat(buf, " vorpal");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
    if (weapon_flags & WEAPON_SHOCKING  ) strcat(buf, " shocking");
    if (weapon_flags & WEAPON_POISON    ) strcat(buf, " poison");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *cont_bit_name( int cont_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (cont_flags & CONT_CLOSEABLE     ) strcat(buf, " closable");
    if (cont_flags & CONT_PICKPROOF     ) strcat(buf, " pickproof");
    if (cont_flags & CONT_CLOSED        ) strcat(buf, " closed");
    if (cont_flags & CONT_LOCKED        ) strcat(buf, " locked");

    return (buf[0] != '\0' ) ? buf+1 : "none";
}


char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK     ) strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB        ) strcat(buf, " backstab");
    if (off_flags & OFF_BASH            ) strcat(buf, " bash");
    if (off_flags & OFF_BERSERK         ) strcat(buf, " berserk");
    if (off_flags & OFF_DISARM          ) strcat(buf, " disarm");
    if (off_flags & OFF_DODGE           ) strcat(buf, " dodge");
    if (off_flags & OFF_FADE            ) strcat(buf, " fade");
    if (off_flags & OFF_FAST            ) strcat(buf, " fast");
    if (off_flags & OFF_KICK            ) strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT       ) strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY           ) strcat(buf, " parry");
    if (off_flags & OFF_RESCUE          ) strcat(buf, " rescue");
    if (off_flags & OFF_TAIL            ) strcat(buf, " tail");
    if (off_flags & OFF_TRIP            ) strcat(buf, " trip");
    if (off_flags & OFF_CRUSH           ) strcat(buf, " crush");
    if (off_flags & ASSIST_ALL          ) strcat(buf, " assist_all");
    if (off_flags & ASSIST_ALIGN        ) strcat(buf, " assist_align");
    if (off_flags & ASSIST_RACE         ) strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS      ) strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD        ) strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM         ) strcat(buf, " assist_vnum");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}
