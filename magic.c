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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "magic.h"
#include "recycle.h"

/* command procedures needed */
//DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_wear          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_get_obj_world );
DECLARE_DO_FUN(do_scan	        );


#define SPELL_DAM		4

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void    wear_obj        args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool show ) );
ROOM_INDEX_DATA * find_location args( ( CHAR_DATA *ch, char *arg ) );
bool    can_use_nexus   args( (CHAR_DATA *ch, int sn) );

extern char *target_name;

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if ( found == -1)
		found = sn;
	    if (ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}


/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    char buf3 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *  old;
	char *  new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",          " "             },
	{ "ar",         "abra"          },
	{ "au",         "kada"          },
	{ "bless",      "auva"          },
	{ "blind",      "nir"           },
	{ "bur",        "mosa"          },
	{ "cu",         "tus"           },
	{ "de",         "calo"          },
	{ "en",         "unso"          },
	{ "light",      "luma"          },
	{ "lo",         "sil"           },
	{ "mor",        "las"           },
	{ "move",       "sido"          },
	{ "ness",       "lasa"          },
	{ "ning",       "illa"          },
	{ "per",        "dreca"         },
	{ "ra",         "mu"            },
	{ "fresh",      "vi"            },
	{ "re",         "rula"          },
	{ "son",        "sasu"          },
	{ "tect",       "shirra"        },
	{ "tri",        "cula"          },
	{ "ven",        "oura"          },
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "l" }, { "t", "h" },
	{ "u", "a" }, { "v", "z" }, { "w", "s" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]      = '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf3,  "You utter the words, '%s'.", buf );
    act(buf3, ch, NULL, NULL, TO_CHAR);
    
    
    sprintf( buf2, "$n utters the words, '%s'.", buf );
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if (rch->altitude != ch->altitude) continue;
	if (rch->secondaryrace == RACE_WARLOCK && dice(2, 3) < ch->power[DISC_WARL_LUCU] )
	{        
	    sprintf( buf3, "$n casts the spell, '%s'.", skill_table[sn].name);                
	    act( buf3, ch, NULL, rch, TO_VICT);
	}
	else act( buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

    save = 50 + ( victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:         return TRUE;
	case IS_RESISTANT:      save += 2;      break;
	case IS_VULNERABLE:     save -= 2;      break;
    }

    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;  
      /* very hard to dispel permanent effects */

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
	for ( af = victim->affected; af != NULL; af = af->next )
	{
	    if ( af->type == sn )
	    {
		if (!saves_dispel(dis_level,af->level,af->duration))
		{
		    affect_strip(victim,sn);
		    if ( skill_table[sn].msg_off )
		    {
			send_to_char( skill_table[sn].msg_off, victim );
			send_to_char( "\n\r", victim );
		    }
		    return TRUE;
		}
		else
		    af->level--;
	    }
	}
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    return min_mana;
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_pcast( CHAR_DATA *ch, char *argument )
{
    int sn;
    
    if (ch->power[DISC_WARL_LUCU] < 2)
    {
	send_to_char("You have not achieved sufficient mastery in the discipline of lucubration.\n\r", ch);
	return;
    }
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Pretend cast what, and where?\n\r", ch );
	return;
    }

    if ( IS_NEWFLAG(ch, AFF_FEAR) )
    {
	send_to_char( "Your hands are trembling too much to cast a spell.\n\r", ch );
	return;
    }

    if ((sn = find_spell(ch,argument)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    || (!IS_NPC(ch) &&  ch->pcdata->learned[sn] == 0))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }
    
    say_spell( ch, sn );
}


void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *was_room;
    void *vo;
    int sn, j;
    bool controlled = FALSE;
    OBJ_DATA *component;
    int target, realm, level, memorized = -1;
    bool elfcontrolled = FALSE, innate = FALSE;
    int exitdir = 0;

    if (IS_NPC(ch)) return;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }
    if (!str_cmp(GET_PC_NAME(ch), "Mitzrah")) innate = TRUE;

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null)
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }

    if (GOT_INNATE(ch) && CAN_INNATE(ch, sn))
    {
	// use the innate over and above memorised spells
	innate = TRUE;
    }

    if (!innate && ch->cur_form != 0 && !form_data[ch->cur_form].can_cast)
    {
	stc("You can't cast spells in this form.\n\r",ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_MESMERISED && dice(1,2) == 1))
    {
	send_to_char("But there are too many pretty colours.\n\r", ch);
	WAIT_STATE(ch, 8);
	return;
    }

    if (!innate && IS_FLAG4(ch, AFF_SMOTHERED) )
    {
	send_to_char("You cannot cast a spell, as the plants are smothering you.\n\r", ch);
	return;
    }

    if (IS_NEWFLAG(ch, AFF_SHADOW_PLANE))
    {
	send_to_char("You cannot draw upon any magical energies while in this plane of existance.\n\r",ch);
	return;
    }

    if (!innate && ch->rage > 0 && !IS_NPC(ch) && ch->level < 120)
    {
	send_to_char("While your beast is controlling you, it is impossible to concentrate.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char("Your spell fizzles and dies.\n\r", ch);
	return;
    }

    if ( IS_NEWFLAG(ch, AFF_FEAR) )
    {
	send_to_char( "Your hands are trembling too much to cast a spell.\n\r", ch );
	return;
    }

    if (!TIME_UP(ch, TIMER_NEXUS_STUNNED))
    {
	stc("You feel too stunned and disorientated to cast spells.\n\r", ch);
	return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    || ((!IS_NPC(ch) && ch->pcdata->learned[sn] == 0) && !innate))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }

    realm = skill_table[sn].realm;
    level = skill_table[sn].level;

    if (!innate && realm == 1 && !can_cast_flame(ch))  
    {
	send_to_char("You cannot cast flame spells.\n\r", ch);
	return;
    }

    if (!innate && realm == 2 && !can_cast_water(ch))  
    {
	send_to_char("You cannot cast water spells.\n\r", ch);
	return;
    }

    if (!innate && realm == 3 && !can_cast_air(ch))  
    {
	send_to_char("You cannot cast air spells.\n\r", ch);
	return;
    }

    if (!innate && realm == 4 && !can_cast_earth(ch))  
    {
	send_to_char("You cannot cast earth spells.\n\r", ch);
	return;
    }

    if (!innate && realm == 5 && !can_cast_entropy(ch))  
    {
	send_to_char("You cannot cast entropy spells.\n\r", ch);
	return;
    }

    if (!innate && realm == 6 && !can_cast_life(ch))  
    {
	send_to_char("You cannot use life magic.\n\r", ch);
	return;
    }

    if ( !innate && ch->alignment > -500 && skill_table[sn].realm == 5 && ch->level < 120
	&& str_cmp(skill_table[sn].name, "dark pact"))
    {
	send_to_char( "You are too pure to cast such an evil spell.\n\r", ch);
	return;
    }

    /* Checks if spell is memorized by caster */

    if (!innate && !IS_NPC(ch))
    {
        for (j = 0 ; j < 10 ; j++)
	    if (ch->pcdata->memorize[level][j] == sn) memorized = j;

        if (memorized == -1 && ch->level < 112)
        {
	    send_to_char("You do not have that spell memorized.\n\r", ch);
	    return;
        }
    }

    if ( !innate && ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    /*
     * Locate targets.
     */
    victim      = NULL;
    obj         = NULL;
    vo          = NULL;
    target      = TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		if (can_use_nexus(ch, sn))
		{
		    stc("You channel your magical energy into the nexus.\n\r", ch);
		    was_room = ch->in_room;
		    char_from_room(ch);
		    char_to_room(ch, was_room->far_link, 0);
		    if ((victim = get_char_room(ch, target_name)) == NULL)
		    {
			stc("They aren't here.\n\r", ch);
		        char_from_room(ch);
		        char_to_room(ch, was_room, 0);
			return;
		    }
		    char_from_room(ch);
		    char_to_room(ch, was_room, ch->altitude);
		}
		else 
		{
		    send_to_char( "They aren't here.\n\r", ch );
		    return;
		}
	    }
	}

	if ( !IS_NPC(ch) )
	{

	    if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	if (victim->secondaryrace == RACE_ELADRIN
	    && ((victim->power[DISC_ELAD_ANSO] * 15) + 10 - (skill_table[sn].level * 5)) > dice(1, 100) 
	    && victim->tertiaryrace == 0
	    && skill_table[sn].level < 9)
	elfcontrolled = TRUE;
	
	/* Check if the Warlock discipline 'control' provides immunity
	     but warlocks can still blow each others brains out! */

	if (victim->secondaryrace == RACE_WARLOCK
	    && ch->secondaryrace != RACE_WARLOCK)
	{
	    if (skill_table[sn].level == 1
		&& victim->power[DISC_WARL_CONT] >= 3)
		controlled = TRUE;

	    else if (skill_table[sn].level == 2
		&& victim->power[DISC_WARL_CONT] >= 5)
		controlled = TRUE;

	    else if (skill_table[sn].level == 3
	    	&& victim->power[DISC_WARL_CONT] >= 8)
		controlled = TRUE;

	    else if (skill_table[sn].level == 4
	    	&& victim->power[DISC_WARL_CONT] >= 10
	        && dice(1, 9) != 9)
		controlled = TRUE;
	}        
	
	vo = (void *) victim;
	target = TARGET_CHAR;

	/* Ok, this will reverse it muhahahahahahahahah */
	if (IS_FLAG5(victim, AFF5_BACKFIRE) && victim != ch)
  	{
	    REMOVE_BIT(victim->flag5, AFF5_BACKFIRE);
	    vo = (void *) ch;
	    stc("The spell seems to have backfired!\n\r", ch);
	}
	
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		if (can_use_nexus(ch, sn))
		{
		    stc("You channel your magical energy into the nexus.\n\r", ch);
		    was_room = ch->in_room;
		    char_from_room(ch);
		    char_to_room(ch, was_room->far_link, 0);
		    if ((victim = get_char_room(ch, target_name)) == NULL)
		    {
		        char_from_room(ch);
		        char_to_room(ch, was_room, 0);
			stc("They aren't here.\n\r", ch);
			return;
		    }
		    char_from_room(ch);
		    char_to_room(ch, was_room, 0);
		}
		else 
		{
		    send_to_char( "They aren't here.\n\r", ch );
		    return;
		}
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}
	
	vo = (void *) ch;

	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

	    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	    {
		send_to_char( "You can't do that on your own follower.\n\r",
		    ch );
		return;
	    }

	    vo = (void *) victim;
	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
	if (arg2[0] == '\0')
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;                                                 
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;

     case TAR_OBJ_ROOM:
       if ( arg2[0] == '\0' )
       {                               
           send_to_char( "What should the spell be cast upon?\n\r", ch );
           return;
       } 

       if ( ( obj = get_obj_here( ch, target_name) ) == NULL )
       {
           send_to_char( "You can't see that here.\n\r", ch );
           return;
       }

       vo = (void *) obj;                    
       target = TARGET_OBJ;
       break;            

    case TAR_EXIT:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What exit should the spell be cast upon?\n\r", ch );
	    return;
	}

	if (!str_prefix(arg2, "north")) 
	{
	    exitdir = DIR_NORTH;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "east"))
	{
	    exitdir = DIR_EAST;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "south"))
	{
	    exitdir = DIR_SOUTH;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "west"))
	{
	    exitdir = DIR_WEST;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "up"))
	{
	    exitdir = DIR_UP;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "down"))
	{
	    exitdir = DIR_DOWN;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else
	{
	    send_to_char("You may only cast this spell north, west, east, south, up or down.\n\r", ch);
	    return;
	}

	vo = (void *) &exitdir;
	break;

    }
	    
    component = get_obj_by_location(ch,ITEM_HOLD);

    if (str_cmp(skill_table[sn].material_comp,"") && !IS_IMMORTAL(ch) && !innate)
    {
	if (component==NULL)
	{
	    send_to_char("You lack the proper component for this spell.\n\r",ch);
	    return;
	}

	if (str_cmp(skill_table[sn].material_comp,component->short_descr) ) 
	{
	    send_to_char("You lack the proper component for this spell.\n\r",ch);
	    return;
	}
    }

      
    if (!innate && str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
      
    if (str_cmp(skill_table[sn].material_comp,"")
	&& !IS_IMMORTAL(ch) && !innate)
    {
	act("You draw upon the power of $p.",ch,component,NULL,TO_CHAR);
	if (skill_table[sn].per_used > dice(1, 100))
	{
	    act("It flares brightly and vanishes, consumed by the magic.",ch,component,NULL,TO_CHAR);
	    extract_obj(component);
	}
    }

    if (ch->level < 120)
    {
	if (ch->secondaryrace == 6)
	WAIT_STATE( ch, ( skill_table[sn].beats * (10 - ch->power[DISC_WARL_LUCU] ) )/10 );  
	else if (ch->secondaryrace == 5)
	WAIT_STATE( ch, ( skill_table[sn].beats * (15 - ch->power[DISC_ELAD_SOMA] ) )/15 );  
	else
	WAIT_STATE( ch, ( skill_table[sn].beats ) );    
    }

    /* This bit decides whether the spell comes from the memorized
	repertiore or from some other weird power */

    if (innate && !IS_NPC(ch))
    {
	stc("You feel the power coursing through your body.\n\r", ch);
	USE_INNATE(ch, sn);
    }
    else if (skill_table[sn].level == 1
	     && !IS_NPC(ch)
	     && ch->power[DISC_WARL_CONT] >= 4)
    {
	stc("You draw energy from the ground to power the spell.\n\r", ch);
    }
    else if (skill_table[sn].level == 2
	     && !IS_NPC(ch)
	     && ch->power[DISC_WARL_CONT] >= 6)
    {
	switch( ch->primary )
	{
	    case REALM_FLAME :
		stc("The flames in your eyes dim slightly.\n\r", ch);
		break;
	    case REALM_WATER :
		stc("Your throat feels parched for a moment.\n\r", ch);
		break;
	    case REALM_AIR :
		stc("It feels warm for a brief second.\n\r", ch);
		break;
	    case REALM_EARTH :
		stc("You hear a distant rattling sound.\n\r", ch);
		break;
	    case REALM_ENTROPY :
		stc("White smoke leaks from your fingertips.\n\r", ch);
		break;
	    case REALM_LIFE :
		stc("Black smoke leaks from your fingertips.\n\r", ch);
		break;
	}
    }
    else if (skill_table[sn].level == 3
	     && ch->power[DISC_WARL_CONT] >= 9
	     && !IS_NPC(ch)
	     && skill_table[sn].realm == ch->primary)
    {
	switch( ch->primary )
	{
	    case REALM_FLAME :
		stc("The room feels deathly cold for a brief second.\n\r", ch);
		break;
	    case REALM_WATER :
		stc("Your skin feels flaky and dry for a moment.\n\r", ch);
		break;
	    case REALM_AIR :
		stc("You cough briefly.\n\r", ch);
		break;
	    case REALM_EARTH :
		stc("Your head spins and you feel dizzy, then it passes.\n\r", ch);
		break;
	    case REALM_ENTROPY :
		stc("Satanic thought brew in your subconcious.\n\r", ch);
		break;
	    case REALM_LIFE :
		stc("Pure thoughts flood you with energy.\n\r", ch);
		break;
	}
    }
    else if (!IS_NPC(ch)) ch->pcdata->memorize[level][memorized] = -1;

    if (ch->level < 120 && ch->secondaryrace > 0)
    {
	bool hurt=FALSE;
	switch (ch->secondaryrace)
	{
	    case 1:  if (skill_table[sn].realm==2)
			hurt = TRUE;
			break;
	    case 2:  if (skill_table[sn].realm==2)
			hurt = TRUE;
			break;
	    case 3:  if (skill_table[sn].realm==2 || skill_table[sn].realm==1)
			hurt = TRUE;
			break;
	    case 4:  if (skill_table[sn].realm==1)
			hurt = TRUE;
			break;
	    case 5:  if (skill_table[sn].realm==5)
			hurt = TRUE;
			break;
	}
	if (hurt)
	{
	    send_to_char("Pain courses through your body, as magical energies tear you asunder.\n\r",ch);
	    damage_old(ch,ch,dice(skill_table[sn].level,30),sn,DAM_FIRE,FALSE);
	}
    }

    /* made spells easier to cast.. heh */

    if (!innate && ( number_percent( )  - dice(3, 15) > UMAX(get_skill(ch,sn), 10)
	|| (IS_NEWFLAG(ch, AFF_SHIVER) && dice(1,10)>5) ))
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	check_improve(ch,sn,FALSE,1);
    }
    else
    {
	if ( (controlled) && skill_table[sn].realm == 1)
	{
	    act("Smoke pours out of $n's fingertips.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("Smoke pours out of the end of your fingertips, the spell is wasted.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn].realm == 2)
	{
	    act("A shield of water materialises in front of $N and blocks the spell.", ch, NULL, victim, TO_CHAR);
	}
	if ( (controlled) && skill_table[sn].realm == 3)
	{
	    act("$n's spell explodes into orange bubbles which promptly vanish.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("The magic energy turns into little orange bubbles which promptly vanish.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn].realm == 4)
	{
	    act("$n's spell explodes into lots of little pink butterflies which fly away.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("Your spell summons lots of little pink butterflies which then fly away.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn].realm == 5)
	{
	    act("An imp appears from nowhere and deflects $n's spell.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("An imp appears from nowhere and deflects your spell.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn].realm == 6)
	{
	    act("A baby angel appears from nowhere and deflects $n's spell.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("A baby angel appears from nowhere and deflects your spell.\n\r", ch);
	}
	
	if (elfcontrolled)
	{
	    act("Your spell is dissipated harmlessly on contact with $N.", ch, NULL, victim, TO_CHAR);
	    act("You are totally unaffected by $n's spell.", ch, NULL, victim, TO_VICT);
	    act("$n's spell washes over $N, leaving $M unaffected.", ch, NULL, victim, TO_NOTVICT);
	}
	
	if (controlled || elfcontrolled) return;

	/* Lets see if they are casting against a drow! */
	if ( target == TARGET_CHAR
	     &&	get_drow_magic_resistance(victim) >= dice(1, 100) )
  	{
	    act("Your magic resistance absorbs $n's spell.", ch, NULL, victim, TO_VICT);
	    act("$N's magic resistance absorbs $n's spell.", ch, NULL, victim, TO_NOTVICT);
	    if (ch != victim) act("$N's magic resistance absorbs your spell.", ch, NULL, victim, TO_CHAR);
	    else stc("Your magic resistance absorbs your spell.\n\r", ch);
	    return;
	}

	if (IS_NPC(ch)) 
	/* class has spells */
	    (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
	else
	    (*skill_table[sn].spell_fun) (sn, 3 * ch->level/4, ch, vo,target);
	check_improve(ch,sn,TRUE,1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch
    &&   IS_NPC(victim))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;


	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;

	    if ( victim == vch && victim->fighting == NULL )
	    {
		set_fighting( victim, ch );
		break;
	    }
	}
    }

    return;
}

bool can_use_nexus(CHAR_DATA *ch, int sn)
{
    int power, level, realm;
 
    crash("can_use_nexus");
 
    if (ch->in_room == NULL) return FALSE;

    if (ch->in_room->far_link == NULL) return FALSE;

    if (ch->altitude != 0) return FALSE;

    power = ch->power[DISC_WARL_FARR];
    level = skill_table[sn].level;
    realm = skill_table[sn].realm;

    if (power < 2) return FALSE;
 
    if (power == 2 && level > 1) return FALSE;
    if (power == 3 && level > 2) return FALSE;
    if (power == 4 && level > 2) return FALSE;
    if (power == 5 && level > 3) return FALSE;
    if (power == 6 && level > 3) return FALSE;
    if (power == 7 && level > 4) return FALSE;
    if (power == 7 && level == 4 && realm != ch->primary) return FALSE;

    if (power > 7 && level > 5) return FALSE;
    if (power > 7 && level == 4 && realm != ch->primary) return FALSE;
    if (power > 7 && level == 5 && realm != ch->primary) return FALSE;

    return TRUE;
}


void do_revcast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int sn, j, sn2;
    bool controlled = FALSE;
    OBJ_DATA *component;
    int target, realm, level, memorized = -1;
    bool elfcontrolled = FALSE;
    int exitdir = 0;

    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Reverse which spell where?\n\r", ch );
	return;
    }
 
    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null)
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }

    if (ch->cur_form != 0 && !form_data[ch->cur_form].can_cast)
    {
	stc("You can't cast spells in this form.\n\r",ch);
	return;
    }

    if (ch->power[DISC_WARL_CONT] == 1)
    {
	if (ch->faith == 0)
 	{
	    stc("It costs one faith point to reverse a spell.\n\r", ch);
	    return;
 	}
	ch->faith -= 1;
    }

    if (!TIME_UP(ch, TIMER_MESMERISED && dice(1,2) == 1))
    {
	send_to_char("But there are too many pretty colours.\n\r", ch);
	WAIT_STATE(ch, 8);
	return;
    }

    if (!TIME_UP(ch, TIMER_NEXUS_STUNNED))
    {
	stc("You feel too stunned and disorientated to cast spells.\n\r", ch);
	return;
    }

    if (IS_FLAG4(ch, AFF_SMOTHERED) )
    {
	send_to_char("You cannot cast a spell, as the plants are smothering you.\n\r", ch);
	return;
    }

    if (IS_NEWFLAG(ch, AFF_SHADOW_PLANE))
    {
	send_to_char("You cannot draw upon any magical energies while in this plane of existance.\n\r",ch);
	return;
    }

    if (ch->rage > 0 && !IS_NPC(ch) && ch->level < 120)
    {
	send_to_char("While your beast is controlling you, it is impossible to concentrate.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char("Your spell fizzles and dies.\n\r", ch);
	return;
    }

    if ( IS_NEWFLAG(ch, AFF_FEAR) )
    {
	send_to_char( "Your hands are trembling too much to cast a spell.\n\r", ch );
	return;
    }

    if ((sn = find_spell(ch,arg1)) < 1
    ||  skill_table[sn].spell_fun == spell_null
    || (!IS_NPC(ch) && ch->pcdata->learned[sn] == 0))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }

    if (skill_table[sn].rev_name[0] == '\0')
    {
	stc("This spell has no reverse.\n\r", ch);
	return;
    }

    realm = skill_table[sn].realm;
    level = skill_table[sn].level;

    /* Checks if spell is memorized by caster */

    for (j = 0 ; j < 10 ; j++)
	if (ch->pcdata->memorize[level][j] == sn) memorized = j;

    if (memorized == -1 && ch->level < 112)
    {
	send_to_char("You do not have that spell memorized.\n\r", ch);
	return;
    }

    /* Ok.. this is the spell conversion to the reversed version */
    sn2 = skill_lookup(skill_table[sn].rev_name);


    if ( ch->position < skill_table[sn2].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    /*
     * Locate targets.
     */
    victim      = NULL;
    obj         = NULL;
    vo          = NULL;
    target      = TARGET_NONE;
      
    switch ( skill_table[sn2].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}


	if ( !IS_NPC(ch) )
	{

	    if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	}

	if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}
		
	if (victim->secondaryrace == RACE_ELADRIN
	    && ((victim->power[DISC_ELAD_ANSO] * 15) + 10 - (skill_table[DISC_ELAD_ANSO].level * 5)) > dice(1, 100) 
	    && skill_table[sn2].level < 9)
	elfcontrolled = TRUE;
	
	/* Check if the Warlock discipline 'control' provides immunity
	     but warlocks can still blow each others brains out! */

	if (victim->secondaryrace == RACE_WARLOCK
	    && ch->secondaryrace != RACE_WARLOCK)
	{
	    if (skill_table[sn].level == 1
		&& victim->power[DISC_WARL_CONT] >= 3)
		controlled = TRUE;

	    else if (skill_table[sn].level == 2
		&& victim->power[DISC_WARL_CONT] >= 5)
		controlled = TRUE;

	    else if (skill_table[sn].level == 3
	    	&& victim->power[DISC_WARL_CONT] >= 8)
		controlled = TRUE;

	    else if (skill_table[sn].level == 4
	    	&& victim->power[DISC_WARL_CONT] >= 10
	        && dice(1, 9) != 9)
		controlled = TRUE;
	}        
	
	vo = (void *) victim;
	target = TARGET_CHAR;
	
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}
	
	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

	    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	    {
		send_to_char( "You can't do that on your own follower.\n\r",
		    ch );
		return;
	    }

	    vo = (void *) victim;
	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
	if (arg2[0] == '\0')
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;                                                 
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;

     case TAR_OBJ_ROOM:
       if ( arg2[0] == '\0' )
       {                               
           send_to_char( "What should the spell be cast upon?\n\r", ch );
           return;
       } 

       if ( ( obj = get_obj_here( ch, target_name) ) == NULL )
       {
           send_to_char( "You can't see that here.\n\r", ch );
           return;
       }

       vo = (void *) obj;                    
       target = TARGET_OBJ;
       break;            

    case TAR_EXIT:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What exit should the spell be cast upon?\n\r", ch );
	    return;
	}

	if (!str_prefix(arg2, "north")) 
	{
	    exitdir = DIR_NORTH;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "east"))
	{
	    exitdir = DIR_EAST;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "south"))
	{
	    exitdir = DIR_SOUTH;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "west"))
	{
	    exitdir = DIR_WEST;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "up"))
	{
	    exitdir = DIR_UP;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else if (!str_prefix(arg2, "down"))
	{
	    exitdir = DIR_DOWN;
	    if (ch->in_room->exit[exitdir] == NULL)
	    {
		send_to_char("There is no exit in that direction.\n\r", ch);
		return;
 	    }
	}
	else
	{
	    send_to_char("You may only cast this spell north, west, east, south, up or down.\n\r", ch);
	    return;
	}

	vo = (void *) &exitdir;
	break;

    }
	    
    component = get_obj_by_location(ch,ITEM_HOLD);

    if (str_cmp(skill_table[sn].material_comp,"") && !IS_IMMORTAL(ch))
    {
	if (component==NULL)
	{
	    send_to_char("You lack the proper component for this spell.\n\r",ch);
	    return;
	}

	if (str_cmp(skill_table[sn].material_comp,component->short_descr) ) 
	{
	    send_to_char("You lack the proper component for this spell.\n\r",ch);
	    return;
	}
    }

    if (str_cmp(skill_table[sn].material_comp,"")
	&& !IS_IMMORTAL(ch))
    {
	act("You draw upon the power of $p.",ch,component,NULL,TO_CHAR);
	if (skill_table[sn].per_used > dice(1, 100))
	{
	    act("It flares brightly and vanishes, consumed by the magic.",ch,component,NULL,TO_CHAR);
	    extract_obj(component);
	}
    }

    if (ch->level < 120)
    {
	if (ch->secondaryrace == 6)
	WAIT_STATE( ch, ( skill_table[sn2].beats * (10 - ch->power[DISC_WARL_LUCU] ) )/10 ); 
	else if (ch->secondaryrace == 5)
	WAIT_STATE( ch, ( skill_table[sn2].beats * (15 - ch->power[DISC_ELAD_SOMA] ) )/15 );  
	else
	WAIT_STATE( ch, ( skill_table[sn2].beats ) );    
    }


    /* This bit decides whether the spell comes from the memorized
	repertiore or from some other weird power */

    if (skill_table[sn2].level == 1
	     && ch->power[DISC_WARL_CONT] >= 4)
    {
	stc("You draw energy from the ground to power the spell.\n\r", ch);
    }
    else if (skill_table[sn2].level == 2
	     && ch->power[DISC_WARL_CONT] >= 6)
    {
	switch( ch->primary )
	{
	    case REALM_FLAME :
		stc("The flames in your eyes dim slightly.\n\r", ch);
		break;
	    case REALM_WATER :
		stc("Your throat feels parched for a moment.\n\r", ch);
		break;
	    case REALM_AIR :
		stc("It feels warm for a brief second.\n\r", ch);
		break;
	    case REALM_EARTH :
		stc("You hear a distant rattling sound.\n\r", ch);
		break;
	    case REALM_ENTROPY :
		stc("White smoke leaks from your fingertips.\n\r", ch);
		break;
	    case REALM_LIFE :
		stc("Black smoke leaks from your fingertips.\n\r", ch);
		break;
	}
    }
    else if (skill_table[sn2].level == 3
	     && ch->power[DISC_WARL_CONT] >= 9
	     && skill_table[sn2].realm == ch->primary)
    {
	switch( ch->primary )
	{
	    case REALM_FLAME :
		stc("The room feels deathly cold for a brief second.\n\r", ch);
		break;
	    case REALM_WATER :
		stc("Your skin feels flaky and dry for a moment.\n\r", ch);
		break;
	    case REALM_AIR :
		stc("You cough briefly.\n\r", ch);
		break;
	    case REALM_EARTH :
		stc("Your head spins and you feel dizzy, then it passes.\n\r", ch);
		break;
	    case REALM_ENTROPY :
		stc("Satanic thoughts brew in your subconcious.\n\r", ch);
		break;
	    case REALM_LIFE :
		stc("Pure thoughts flood you with energy.\n\r", ch);
		break;
	}
    }
    else ch->pcdata->memorize[level][memorized] = -1;


    /* made spells easier to cast.. heh */

    if (( number_percent( )  - dice(1, 20) > UMAX(get_skill(ch,sn), 10)
	|| (IS_NEWFLAG(ch, AFF_SHIVER) && dice(1,10)>5) ))
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	check_improve(ch,sn,FALSE,1);
    }
    else
    {
	if ( (controlled) && skill_table[sn2].realm == 1)
	{
	    act("Smoke pours out of $n's fingertips.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("Smoke pours out of the end of your fingertips, the spell is wasted.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn2].realm == 2)
	{
	    act("A shield of water materialises in front of $N and blocks the spell.", ch, NULL, victim, TO_CHAR);
	}
	if ( (controlled) && skill_table[sn2].realm == 3)
	{
	    act("$n's spell explodes into orange bubbles which promptly vanish.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("The magic energy turns into little orange bubbles which promptly vanish.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn2].realm == 4)
	{
	    act("$n's spell explodes into lots of little pink butterflies which fly away.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("Your spell summons lots of little pink butterflies which then fly away.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn2].realm == 5)
	{
	    act("An imp appears from nowhere and deflects $n's spell.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("An imp appears from nowhere and deflects your spell.\n\r", ch);
	}
	if ( (controlled) && skill_table[sn].realm == 6)
	{
	    act("A baby angel appears from nowhere and deflects $n's spell.", ch, NULL, NULL, TO_ROOM);
	    send_to_char("A baby angel appears from nowhere and deflects your spell.\n\r", ch);
	}

	if (elfcontrolled)
	{
	    act("Your spell is dissipated harmlessly on contact with $N.", ch, NULL, victim, TO_CHAR);
	    act("You are totally unaffected by $n's spell.", ch, NULL, victim, TO_VICT);
	    act("$n's spell washes over $N, leaving $M unaffected.", ch, NULL, victim, TO_NOTVICT);
	}
	
	if (controlled || elfcontrolled) return;

	/* Lets see if they are casting against a drow! */
	if ( target == TARGET_CHAR
	     &&	get_drow_magic_resistance(victim) >= dice(1, 100) )
  	{
	    act("Your magic resistance absorbs $n's spell.", ch, NULL, victim, TO_VICT);
	    act("$N's magic resistance absorbs $n's spell.", ch, NULL, victim, TO_NOTVICT);
	    if (ch != victim) act("$N's magic resistance absorbs your spell.", ch, NULL, victim, TO_CHAR);
	    else stc("Your magic resistance absorbs your spell.\n\r", ch);
	    return;
	}

	if (IS_NPC(ch)) 
	/* class has spells */
	    (*skill_table[sn2].spell_fun) ( sn, ch->level, ch, vo,target);
	else
	    (*skill_table[sn2].spell_fun) (sn, 3 * ch->level/4, ch, vo,target);
	check_improve(ch,sn2,TRUE,1);
    }


    if ((skill_table[sn2].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn2].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch
    &&   IS_NPC(victim))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;

	    if ( victim == vch && victim->fighting == NULL )
	    {
		set_fighting( victim, ch );
		break;
	    }
	}
    }

    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if ( victim == NULL && obj == NULL)
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (victim != NULL)
	    {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Something isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
		vo = (void *) obj;
		target = TARGET_OBJ;
	    }
	break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		set_fighting( victim, ch );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */
void spell_magma_burst( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    send_to_char("A stream of molten magma shoot from your outstretched palms.\n\r",ch);
    send_to_char("You are hit with a stream of molten magma.\n\r", victim);
    dam = dice( level, 16 )/ 6 * ch->fire * SPELL_DAM;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	    send_to_char("Your magma is cooled by the water.\n\r", ch);
	    dam /= 3;    
    }
    if (ch->fire == 10)
	dam *=2;
    if ( saves_spell( level, victim, DAM_FIRE ) )
	dam /= 3;
    damage_old( ch, victim, dam, sn,DAM_FIRE,TRUE);
    return;
}

void spell_lava_burst( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    send_to_char("Streams of molten lava shoot from your outstretched palms.\n\r",ch);
    send_to_char("You are sprayed with molten lava.\n\r", victim);
    dam = dice( level, 20 )/ 6 * ch->fire * SPELL_DAM;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	    send_to_char("Your lava is cooled by the water.\n\r", ch);
	    dam /= 3;    
    }
    if (ch->fire == 9)
	dam *=2;
    if (ch->fire == 10)
	dam *=2;
    if ( saves_spell( level, victim, DAM_FIRE ) )
	dam /= 5;
    damage_old( ch, victim, dam, sn,DAM_FIRE,TRUE);
    return;
}

void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    send_to_char("Acid shoots from the tip of your finger.\n\r",ch);
    send_to_char("You are covered with burning acid.\n\r", victim);

    dam = dice( (ch->earth * 2), 10 )/6 * ch->earth * SPELL_DAM;
    if (ch->earth == 9)
	dam *=3 / 2;
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage_old( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}



void spell_windtomb( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You have that spell on you already.\n\r",ch);
	else
	  act("$N is already within a windtomb.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->air * 4;
    af.modifier  = -40 - (ch->air * 2);
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You are enveloped within a roaring hurricane.\n\r", victim );
    act("A roaring wind envelops $n.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_glitterdust( int sn, int level, CHAR_DATA *ch, void *vo, int target )

{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	     send_to_char("You have already got that spell on you!\n\r",ch);
	else
	act("$N is already affected.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (ch->earth * 5) ;
    af.modifier  = -1;
    if ( ch->earth > 6)
	af.modifier = -2;
    if (af.duration < 10)
	af.duration = 10;
    af.location  = APPLY_DEX;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Small motes of light circle around you.\n\r", victim );
    act("Small globes of light swirl around $n's head, disorientating $m.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are armoured already.\n\r",ch);
	else
	  act("$N is already armoured.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (ch->earth * 6) ;
    af.modifier  = 0 - (ch->earth * 4);
    if (af.modifier > -20)
	af.modifier = -20;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Small motes of light circle around you.\n\r", victim );
    act("Glowing motes of light start to circle around $n.", victim, NULL, NULL, TO_ROOM);
    return;
}


void spell_grounding( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SET( victim->imm_flags, IMM_LIGHTNING ) )
    {
	if (victim == ch)
	  send_to_char("You are already immune to electricity.\n\r",ch);
	else
	  act("$N is already immune.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_IMMUNE;
    af.type      = sn;
    af.level     = level;
    af.duration  = (ch->earth * 2);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = IMM_LIGHTNING;
    affect_to_char( victim, &af );
    send_to_char( "You channel all electrical attacks into the ground.\n\r", victim );
    act("$n lays $s hands on the ground and starts chanting.",ch,NULL,NULL,TO_ROOM);
    return;
}

void spell_storm_herald( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SET( victim->vuln_flags, VULN_LIGHTNING ) )
    {
	if (victim == ch)
	  send_to_char("You are already vulnerable to electricity.\n\r",ch);
	else
	  act("$N is already affected.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_VULN;
    af.type      = sn;
    af.level     = level;
    af.duration  = (ch->air * 2);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = VULN_LIGHTNING;
    affect_to_char( victim, &af );
    send_to_char( "You channel all electrical attacks into your body.\n\r", victim );
    act("$n fizzles and sparks with energy.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_resist_cold( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_SET( victim->imm_flags, IMM_COLD ) || IS_SET( victim->res_flags, RES_COLD) )
    {
	if (victim == ch)
	  send_to_char("You are already resistant to cold.\n\r",ch);
	else
	  act("$N is already resistant to cold.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = (ch->water * 3);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = RES_COLD;
    affect_to_char( victim, &af );
    send_to_char( "You are now more resistant to cold.\n\r", victim );
    act("$n's skin turns a pale blue.",victim,NULL,NULL,TO_ROOM);
    return;
}




void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,gsn_curse);
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return;
	    }
	    else
	    {
		act("The evil of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}
	
	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = 6 + level;
	af.location     = APPLY_SAVES;
	af.modifier     = -1;
	af.bitvector    = ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw -= 1;
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already blessed.\n\r",ch);
	else
	  act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->secondaryrace == RACE_VAMPIRE)
    {
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->water * 5;
    if (af.duration < 12)
	af.duration = 12;
    af.location  = APPLY_HITROLL;
    af.modifier  = 0 - (level / (11 - ch->water));
    if (af.modifier < 5)
	af.modifier = 5;

    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel cursed by a blessing.\n\r", victim );
    if ( ch != victim )
	act("You smite $N with a bless from the gods.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->water * 5;
    if (af.duration < 12)
	af.duration = 12;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / (11 - ch->water);
    if (af.modifier < 5)
	af.modifier = 5;

    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
	act("You grant $N the favor of your diety.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_cry_of_anger( int sn, int level, CHAR_DATA *ch, void *vo, int target) {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;


    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already frenzied enough.\n\r",ch);
	else
	  act("$N already has this spell.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (level / 8) + (ch->fire * 4);
    af.location  = APPLY_HITROLL;
    af.modifier  = (level / 6) + (ch->fire * 2);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = (level / 6) + (ch->fire * 2);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel the urge to fight.\n\r", victim );
    if ( ch != victim )
	act("You grant $N mastery in battle.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_clfe( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("The fire elemental screams and dies in a cloud of boiling steam.\n\r", ch);
	return;
    }
    victim = create_mobile( get_mob_index( 1 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("$n has summoned a lesser fire elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a lesser fire elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->affected_by, AFF_CHARM);
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    victim->leader = ch;
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_clwe( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 2 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    if (ch->in_room->sector_type == SECT_UNDERWATER)    
    {
	send_to_char("The water elemental is strengthened by its surroundings.\n\r", ch);
	victim->max_hit *= 3 / 2;
	victim->hit *= 3 / 2;
    }
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    act("$n has summoned a lesser water elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a lesser water elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_clae( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 3 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("$n has summoned a lesser air elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a lesser air elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_clee( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 4 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("$n has summoned a lesser earth elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a lesser earth elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_cgfe( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("The fire elemental screams and dies in a cloud of boiling steam.\n\r", ch);
	return;
    }
    victim = create_mobile( get_mob_index( 5 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("$n has summoned a greater fire elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a greater fire elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->affected_by, AFF_CHARM);
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    victim->leader = ch;
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_cgwe( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 6 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    if (ch->in_room->sector_type == SECT_UNDERWATER)    
    {
	send_to_char("The water elemental is strengthened by its surroundings.\n\r", ch);
	victim->max_hit *= 3 / 2;
	victim->hit *= 3 / 2;
    }
    act("$n has summoned a greater water elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a greater water elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_cgae( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 7 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("$n has summoned a greater air elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a greater air elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_cgee( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 8 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("$n has summoned a greater earth elemental.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You summon a greater earth elemental.\n\r",ch);
    add_follower( victim, ch );
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    SET_BIT(victim->flag4, AFF_ELEMENTAL);
}    

void spell_evards_tenticles( int sn, int level, CHAR_DATA *ch, void *vo, int target) 
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 9 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    act("As $n completes $s spell, a rubbery mass of writhing tentacles\n\rburst through the ground.",ch,NULL,NULL,TO_ROOM);
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    send_to_char("A rubbery mass of writhing tentacles burst through the ground.\n\r",ch);
//    do_berserk(victim, "");
}    

void spell_wyvern_watch( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;

    victim = create_mobile( get_mob_index( 13 ) );
    char_to_room(victim, ch->in_room, ch->altitude);
    SET_BIT(victim->flag2, AFF_WATER_BREATHING);
    act("As $n completes $s spell, a spectral wyvern forms in the room.",ch,NULL,NULL,TO_ROOM);
    send_to_char("A shimmering wyvern fades into existence.\n\r",ch);
}    

void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_OTHER))
	return;


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -2 - ch->entropy;
    af.duration  = 12 + ch->entropy;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "Aargh! You are blinded! You cannot see a thing.\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}


void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
    int dam;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0, 14,     17, 20, 23, 26, 29,
	29, 29, 30, 30, 31,     31, 32, 32, 33, 33,
	34, 34, 35, 35, 36,     36, 37, 37, 38, 38,
	39, 39, 40, 40, 41,     41, 42, 42, 43, 43,
	44, 44, 45, 45, 46,     46, 47, 47, 48, 48
    };
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("Your spell explodes in a cloud of steam.\n\r", ch);
	return;
    }

    send_to_char("A gout of flame shoots forth from your hands.\n\r",ch);
    send_to_char("A roaring wall of flame envelops you.\n\r", victim);

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 4, dam_each[level] * 3 / 2 ) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    dam += (ch->fire * 2);
    damage_old( ch, victim, dam, sn, DAM_FIRE,TRUE);
    return;
}



void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be outside to call lightning.\n\r", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "There isn't a storm!\n\r", ch );
	return;
    }

    dam = dice(level/2 + ch->air, ch->air + 5) * SPELL_DAM;

    send_to_char( "Bolts of lightning streak from the sky!\n\r", ch );
    act( "$n calls down lightning to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next        = vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		damage_old( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return;
}

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->altitude != ch->altitude) continue;
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	if (vch->altitude != ch->altitude) continue;
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;
	    
	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
	(IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 
    if (check_dispel(level,victim,skill_lookup("armor")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("jump")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("regeneration")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("stone grip")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("hideous laughter")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("storm herald")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("improved invisibility")))
	found = TRUE;
    
    if (check_dispel(level,victim,skill_lookup("reflection")))
	found = TRUE;
    
    if (check_dispel(level,victim,skill_lookup("deafness")))
	found = TRUE;
    
    if (check_dispel(level,victim,skill_lookup("grounding")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("hold person")))
    {
	found = TRUE;
	act("$n can move again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
	found = TRUE;
	act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
	found = TRUE;
	act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
	found = TRUE;
	act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
	found = TRUE;
	act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("glitterdust")))
    {
	found = TRUE;
	act("The glowing balls of light disappear.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("hold person")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("water breathing")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("time twister")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
	act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("conflaguration")))
    {
	act("The flames around $n fade.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
	act("$n falls to the ground!  Ouch!",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
	act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n stops moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
	act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
	act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection evil")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
	found = TRUE; 
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
	act("The white aura around $n's body vanishes.",
	    victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
	act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
	act("$n stops moving so slowly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
	act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("iron skin")))
    {
	act("$n's skin looks softer.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
	act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
    
    check_fall(victim);
 
    if (!found)
	send_to_char("The spell did not seem to work.\n\r",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    damage_old( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3 * SPELL_DAM, sn,DAM_HARM,TRUE);
    return;
}



void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    send_to_char("You rend your victim apart with this evil sorcery.\n\r",ch);
    send_to_char("Your bones twist and your muscles contort in agony.\n\r", victim);
    damage_old( ch, (CHAR_DATA *) vo, dice(ch->entropy, 8) * SPELL_DAM + level - 6, sn,DAM_HARM,TRUE);
    return;
}



void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage_old( ch, (CHAR_DATA *) vo, dice(2, 8) * SPELL_DAM + level / 2 + ch->entropy, sn,DAM_HARM,TRUE);
    return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("Your lightning bolt is conducted by the water around you.\n\r", ch);
	damage_old(ch, ch, 260, sn, DAM_LIGHTNING, TRUE);
	return;
    }

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
	ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

    dam = dice(level,6) * SPELL_DAM;
    if (saves_spell(level,victim,DAM_LIGHTNING))
	dam /= 3;
    damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
	if (tmp_vict->altitude > ch->altitude + 1) continue;
	if (tmp_vict->altitude < ch->altitude - 1) continue;

	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	    act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	    dam = dice(level,6);
	    if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
		dam /= 3;
	    damage_old(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	    level -= 4;  /* decrement damage */
	  }
	}   /* end target searching loop */
	
	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
	    return;

	  if (last_vict == ch) /* no double hits */
	  {
	    act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	    act("The bolt grounds out through your body.",
		ch,NULL,NULL,TO_CHAR);
	    return;
	  }
	
	  last_vict = ch;
	  act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You are struck by your own lightning!\n\r",ch);
	  dam = dice(level,6);
	  if (saves_spell(level,ch,DAM_LIGHTNING))
	    dam /= 3;
	  damage_old(ch,ch,dam,sn,DAM_LIGHTNING,TRUE);
	  level -= 4;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
    /* now go back and find more targets */
    }
}
	  
void spell_dark_pact(int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    if (ch->alignment == -1000)
    {
	send_to_char("You have no soul left to sell.\n\r",ch);
	return;
    }

    ch->alignment -= 300;
    if (ch->alignment < -1000) ch->alignment = -1000;
    send_to_char("You give some of your soul to the devil.\n\r",ch);
}

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level , victim,DAM_OTHER))
	return; 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->air * 6;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your sex has changed, how kinky!\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_reset( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
  
    if (victim->cur_form == get_normal_form(victim))
	{
	send_to_char("They are already in their original form.\n\r",ch);
	return;
	}
    if (saves_spell(level - ch->water + victim->water , victim,DAM_OTHER) && ch != victim) 
	{
	send_to_char("The spell fizzles and dies.\n\r",ch);
	return; 
	}

    send_to_char("The spell was successful.\n\r", ch);
    send_to_char("You change back into your original form.\n\r", victim);
    set_form(victim, get_normal_form(victim) );
}       


void spell_trip( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
  
    if (saves_spell( ( level / 2 ) , victim,DAM_OTHER) && ch != victim) 
	{
	send_to_char("Your opponent dodged your spell.\n\r",ch);
	return; 
	}

    send_to_char("You trip, and fall on the floor.\n\r", victim);
    if (ch != victim)
	send_to_char("Your opponent trips and stumbles.\n\r", ch);
    set_stance(victim, 0);
}       



void spell_cockroach( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
  
    if ( victim->cur_form == 8)
    {
	if (victim == ch)
	  send_to_char("You are already a cockroach.\n\r",ch);
	else
	  act("$N is already a cockroach.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level - ch->entropy , victim,DAM_OTHER))
	{
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return; 
	}
    send_to_char( "You start to shrink and change form until ...\n\r", victim );
    send_to_char( "ARGH! You have changed into a cockroach.\n\r", victim );
    act("$n turns into a nasty little cockroach.",victim,NULL,NULL,TO_ROOM);
    set_form( victim , 8);      
    return;
}

void spell_frog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
  
    if ( victim->cur_form == 11)
    {
	if (victim == ch)
	  send_to_char("You are already a frog.\n\r",ch);
	else
	  act("$N is already a frog.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level - ch->earth , victim,DAM_OTHER))
	{
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return; 
	}

    send_to_char( "You start to shrink and change form until ...\n\r", victim );
    send_to_char( "ARGH! You have changed into a nasty little frog.\n\r", victim );
    act("$n turns into a nasty little frog.",victim,NULL,NULL,TO_ROOM);
    set_form( victim , FRM_FROG);     
    return;
}

void spell_lure_of_darkness( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if(saves_spell(level , victim, DAM_NEGATIVE))
    {
	send_to_char("You fail to lure your victim.\n\r",ch);
	return;   
    } 

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if (ch == victim)
    {
	send_to_char("Your own soul is already corrupted.\n\r", ch);
	return;
    }

    send_to_char( "Dark thoughts brew in your subconcious.\n\r", victim );
    act("You have successfully lured $N onto the path of evil.",ch,NULL,victim,TO_CHAR);
    victim->alignment -= 1;
    if (victim->alignment < -1000)
	victim->alignment = -1000;        
    return;
}

void spell_hunger(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    
    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if (saves_spell(level , victim,DAM_NEGATIVE))
    {
	send_to_char("Your spell fizzles and dies.\n\r",ch);
	return;   
    } 

    if (IS_NPC(victim))
    {
	act("$n feels extremely hungry all of a sudden.",victim,NULL,ch,TO_ROOM);
	return;
    }
    send_to_char( "The pangs of hunger grip your stomach.\n\r", victim );
    act("$n feels extremely hungry all of a sudden.",victim,NULL,NULL,TO_CHAR);
    victim->pcdata->condition[COND_HUNGER] = 5;
    return;
}

void spell_thirst( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if (saves_spell(level , victim,DAM_NEGATIVE))
    {
	send_to_char("Your spell fizzles and dies.\n\r",ch);
	return;   
    } 

    if (IS_NPC(victim))
    {
	act("$n feels extremely thirsty all of a sudden.",victim,NULL,ch,TO_CHAR);
	return;
    }

    send_to_char( "Your throat is dry and parched, you feel incredibly thirsty.\n\r", victim );
    act("$n feels extremely thirsty all of a sudden.",victim,NULL,NULL,TO_ROOM); 
    victim->pcdata->condition[COND_THIRST] = 5;
    return;
}


void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_safe(ch,victim)) return;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level - ch->fire, victim,DAM_CHARM) )
	return;


    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  6,  7,  8,      9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,     15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,     19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,     22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,     25, 26, 26, 26, 27
    };
    AFFECT_DATA af;
    int dam;

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] ) * SPELL_DAM;
    dam += (ch->entropy * 2);

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if ( !saves_spell( level, victim,DAM_COLD ) )
    {
	act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }

    damage_old( ch, victim, dam, sn, DAM_COLD,TRUE );
    return;
}



void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,      0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,     55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,     61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,     69, 70, 70, 71, 72,
	73, 73, 74, 75, 76,     76, 77, 78, 79, 79
    };
    int dam;
    send_to_char("A multicoloured fan of light flashes from your open palm.\n\r",ch);
    send_to_char("You are momentarily dazzled by a myriad of colours.\n\r", victim);
    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2,  dam_each[level] * 2 ) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_LIGHT) )
	dam /= 2;
    else 
	spell_blindness(skill_lookup("blindness"),
	    level/2,ch,(void *) victim,TARGET_CHAR);

    damage_old( ch, victim, dam, sn, DAM_LIGHT,TRUE );
    return;
}



void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name,ch);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room, ch->altitude );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}



void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{
    if ( !str_cmp( target_name, "better" ) )
	weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
	weather_info.change -= dice( level / 3, 4 );
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;
    obj_to_room( mushroom, ch->in_room, ch->altitude );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

void spell_create_gourd( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *gourd;

    gourd = create_object( get_obj_index( 113 ), 0 );
    gourd->value[0] = level / 2;
    gourd->value[1] = level;
    obj_to_room( gourd, ch->in_room, ch->altitude );
    act( "$p suddenly appears.", ch, gourd, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, gourd, NULL, TO_CHAR );
    return;
}

void spell_heroes_feast( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mutton;
    OBJ_DATA *mead;
    OBJ_DATA *ambrosia;
    act("$n has created a feast fit for a king.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You have created a feast fit for a king.\n\r", ch);
    mutton = create_object( get_obj_index( 43 ), 0 );
    obj_to_room( mutton, ch->in_room, ch->altitude );
    act( "$p suddenly appears.", ch, mutton, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mutton, NULL, TO_CHAR );
    mead = create_object( get_obj_index( 44 ), 0 );
    obj_to_room( mead, ch->in_room, ch->altitude );
    act( "$p suddenly appears.", ch, mead, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mead, NULL, TO_CHAR );
    ambrosia = create_object( get_obj_index( 45 ), 0 );
    obj_to_room( ambrosia, ch->in_room, ch->altitude );
    act( "$p suddenly appears.", ch, ambrosia, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, ambrosia, NULL, TO_CHAR );
    return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r",ch);
    obj_to_char(rose,ch);
    return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *spring;

    if (ch->secondaryrace == 3)
	spring = create_object( get_obj_index( 29 ), 0 );
    else
	spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room, ch->altitude );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
	if (victim == ch)
	  send_to_char("You aren't blind.\n\r",ch);
	else
	  act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }
 
    if (check_dispel(level,victim,gsn_blindness))
    {
	send_to_char( "Your vision returns!\n\r", victim );
	act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal; 

    heal = dice(3, 8) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) 
       && !is_affected( victim, gsn_pestilence) )
    {
	if (victim == ch)
	  send_to_char("You aren't ill.\n\r",ch);
	else
	  act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
	return;
    }
    
    if (check_dispel(level,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
    if (check_dispel(level,victim,gsn_pestilence))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }

	send_to_char("Spell failed.\n\r",ch);
}

void spell_remove_fear( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_fear ) )
    {
	if (victim == ch)
	  send_to_char("You aren't afraid of anything.\n\r",ch);
	else
	  act("$N doesn't appear to be affected by fear.",ch,NULL,victim,TO_CHAR);
	return;
    }
    
    if (check_dispel(level,victim,gsn_fear))
    {
	send_to_char("Your fears subside.\n\r",victim);
	act("$n looks less frightened.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("The spell fizzles and dies.\n\r",ch);
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(1, 8) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gsn_poison ) )
    {
	if (victim == ch)
	  send_to_char("You aren't poisoned.\n\r",ch);
	else
	  act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
	return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
	send_to_char("A warm feeling runs through your body.\n\r",victim);
	act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, 8) + level /2 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,skill_lookup("bless"));
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
		return;
	    }
	    else
	    {
		act("The holy aura of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}

	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = 2 * level;
	af.location     = APPLY_SAVES;
	af.modifier     = +1;
	af.bitvector    = ITEM_EVIL;
	affect_to_obj(obj,&af);

	act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_NONE)
	    ch->saving_throw += 1;
	return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
	victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);

    if (victim != ch)
    {
	act("$n calls forth the demons of Hell upon $N!",
	    ch,NULL,victim,TO_ROOM);
	act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    dam = dice( level, 10 ) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}


void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
	if (victim == ch)
	  send_to_char("You can already sense good.\n\r",ch);
	else
	  act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
	if (victim == ch)
	  send_to_char("You are already as alert as you can be. \n\r",ch);
	else
	  act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
	if (victim == ch)
	  send_to_char("You can already see invisible.\n\r",ch);
	else
	  act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
	if (victim == ch)
	  send_to_char("You can already sense magical auras.\n\r",ch);
	else
	  act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	act( "The powers smile upon $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
}


void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
	victim = ch;
 
    if ( IS_EVIL(victim) )
    {
	act( "$N is protected by $S evil.", ch, NULL, victim, TO_ROOM );
	return;
    }
 
    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }
 
    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    return;
}


/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim,DAM_OTHER))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("armor")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
	found = TRUE;
	act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
	found = TRUE;
	act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
	found = TRUE;
	act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
	found = TRUE;
	act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("hold person")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("reflection")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("curse")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
	act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("conflaguration")))
    {
	act("The flames around $n fade.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
	act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
	act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
	act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
	act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
	found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("protection evil")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
	found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
	act("The white aura around $n's body vanishes.",
	    victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
	act("The white aura around $n's body vanishes.",
	    victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
	act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
	found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
	act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
	act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
	act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }

    check_fall(victim);
 
    if (found)
	send_to_char("Ok.\n\r",ch);
    else
	send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_bind_daemon( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
    DESCRIPTOR_DATA *d;
    bool got=FALSE,found=TRUE;
    CHAR_DATA *victim;
    AFFECT_DATA af;

    while (!got && found)
    {
    found = FALSE;
    for (d = descriptor_list; d != NULL; d = d->next )
    {
	if (d->connected == CON_PLAYING
	&&  d->character != ch
	&&  d->character->in_room != NULL   
	&&  can_see( ch, d->character )  
	&&  d->character->secondaryrace == 2
	&&  !got )
	{
	    victim = d->character;
	    found = TRUE;
	    if (dice(1, 20) == 10)
    {
    send_to_char("You have successfully bound a daemon.\n\r",ch);
    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "A black portal appears over $n, and $s is sucked away into the void.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room, ch->altitude );
    act( "$n arrives screaming through the blackness of the void.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has performed the rite of summoning and binding on you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level + ch->fire*2, victim,DAM_CHARM) )
    {
	send_to_char("The spell fizzles and dies.  Somwthing has gone wrong.\n\r",ch);
//	do_berserk(victim, "");
	return;
    }
    if (!IS_NEWFLAG(ch, AFF_PENTAGRAM))
    {
	act("$N breaks free of $s bonds, and goes berserk.",ch,NULL,victim,TO_CHAR);
	act("$N breaks free of $s bonds, and goes berserk.",ch,NULL,victim,TO_ROOM);
	send_to_char("You break free of the binding spell, and go berserk.\n\r",victim);
//	do_berserk(victim, "");
	return;
    }
    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2424;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "You follow $n, you must obey, $e is your new master.", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	act("$N is now under your power.",ch,NULL,victim,TO_CHAR);
		got = TRUE;
    }
	}
    }
    }
    if (!got)
    {
	send_to_char("You failed to summon a daemon.\n\r",ch);
	return;  
    }
}





void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next        = vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
		if (is_flying(vch))
		    damage_old(ch,vch,0,sn,DAM_BASH,TRUE);
		else
		    damage_old( ch,vch,(level + dice(2, 8)) * SPELL_DAM, sn, DAM_BASH,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

void spell_star_storm( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "Hundreds of fiery stars burst from the heavens.\n\r", ch );
    act( "$n calls down a swarm of fiery comets from the sky.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next        = vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch )
		damage_old( ch,vch,(level + dice(level, 150)) * SPELL_DAM, sn, DAM_FIRE,TRUE);
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "You hear a huge explosion in the distance.\n\r", vch );
    }
    return;
}

void spell_sunbeam( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The brilliant radiance of the sun fills the room.\n\r", ch );
    act( "$n calls down a beam of sunlight, striking you.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next        = vch->next;



	if ( vch->in_room == NULL )
	    continue;

	if ( vch->in_room == ch->in_room
	      && vch != ch
	      && vch->secondaryrace == RACE_VAMPIRE )
	{
	    if (vch->value[6] == 0) vch->value[6]++;
	    damage_old( ch,vch,(level + dice(ch->fire, 80) * 60 / vch->value[6]) * SPELL_DAM, sn, DAM_FIRE,TRUE);
	}
	else if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "Suddenly, the room is filled with bright sunlight, then it fades.\n\r", vch );
    }
    return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if (obj->item_type != ITEM_ARMOR)
    {
	send_to_char("That isn't an armor.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;  /* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->location == APPLY_AC )
	    {
		ac_bonus = paf->modifier;
		ac_found = TRUE;
		fail += 5 * (ac_bonus * ac_bonus);
	    }

	    else  /* things get a little harder */
		fail += 20;
	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
	{
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
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

    if (result <= (90 - level/5))  /* success! */
    {
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
	act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = -1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = -2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (ac_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_AC)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where      = TO_OBJECT;
	paf->type       = sn;
	paf->level      = level;
	paf->duration   = -1;
	paf->location   = APPLY_AC;
	paf->modifier   =  added;
	paf->bitvector  = 0;
	paf->next       = obj->affected;
	obj->affected   = paf;
    }

}




void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf; 
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if (obj->item_type != ITEM_WEAPON)
    {
	send_to_char("That isn't a weapon.\n\r",ch);
	return;
    }

    if (obj->wear_loc != -1)
    {
	send_to_char("The item must be carried to be enchanted.\n\r",ch);
	return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;  /* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
	    if ( paf->location == APPLY_HITROLL )
	    {
		hit_bonus = paf->modifier;
		hit_found = TRUE;
		fail += 2 * (hit_bonus * hit_bonus);
	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
		dam_bonus = paf->modifier;
		dam_found = TRUE;
		fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
		fail += 25;
	}
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
	{
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
	{
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);

    result = number_percent();

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p shivers violently and explodeds!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 2)) /* item disenchanted */
    {
	AFFECT_DATA *paf_next;

	act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
	obj->enchanted = TRUE;

	/* remove all affects */
	for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	    paf_next = paf->next; 
	    free_affect(paf);
	}
	obj->affected = NULL;

	/* clear all flags */
	obj->extra_flags = 0;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	send_to_char("Nothing seemed to happen.\n\r",ch);
	return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
	AFFECT_DATA *af_new;
	obj->enchanted = TRUE;

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) 
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

    if (result <= (100 - level/5))  /* success! */
    {
	act("$p glows blue.",ch,obj,NULL,TO_CHAR);
	act("$p glows blue.",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	added = 1;
    }
    
    else  /* exceptional enchant */
    {
	act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
	act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    /* now add the enchantments */ 

    if (obj->level < LEVEL_HERO - 1)
	obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

    if (dam_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_DAMROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();

	paf->where      = TO_OBJECT;
	paf->type       = sn;
	paf->level      = level;
	paf->duration   = -1;
	paf->location   = APPLY_DAMROLL;
	paf->modifier   =  added;
	paf->bitvector  = 0;
	paf->next       = obj->affected;
	obj->affected   = paf;
    }

    if (hit_found)
    {
	for ( paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    if ( paf->location == APPLY_HITROLL)
	    {
		paf->type = sn;
		paf->modifier += added;
		paf->level = UMAX(paf->level,level);
		if (paf->modifier > 4)
		    SET_BIT(obj->extra_flags,ITEM_HUM);
	    }
	}
    }
    else /* add a new affect */
    {
	paf = new_affect();
 
	paf->type       = sn;
	paf->level      = level;
	paf->duration   = -1;
	paf->location   = APPLY_HITROLL;
	paf->modifier   =  added;
	paf->bitvector  = 0;
	paf->next       = obj->affected;
	obj->affected   = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_devour_soul( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
	ch->alignment = UMAX(-1000, ch->alignment - 20);

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if (victim->secondaryrace == 0)
    {
	stc("Black vapours form around your hands then die away.\n\r", ch);
	act("Black smoke pours out of $n's hands.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a sharp pain in your chest.\n\r",victim);        
	damage_old( ch, victim, 20*ch->entropy*SPELL_DAM, sn, DAM_NEGATIVE ,TRUE);
	return;
    }


    if ( victim->level <= 2 )
    {
	dam              = ch->hit + 1;
    }
    else
    {
	victim->move    /= 3;
	dam              = dice(ch->entropy, level);
	ch->hit         += dam;
    }

    send_to_char("Your soul is wrenched from your chest.\n\r",victim);
    send_to_char("You feed on the life force of your victim.\n\r",ch);
    damage_old( ch, victim, dam*SPELL_DAM, sn, DAM_NEGATIVE ,TRUE);

    return;
}

void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
	ch->alignment = UMAX(-1000, ch->alignment - 10);

    if (victim->power[DISC_DAEM_NETH] >= 4)
    {
	stc("Your spell dissipates in a puff of netherous smoke.\n\r", ch);
	stc("The spell is absorbed by your nether shield.\n\r", victim);
	act("Black wisps of smoke absorb $n's spell.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if (victim->secondaryrace == 0)
    {
	stc("Black vapours form around your hands then die away.\n\r", ch);
	act("Black smoke pours out of $n's hands.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);         
	return;
    }


    if ( victim->level <= 2 )
    {
	dam              = ch->hit + 1;
    }
    else
    {
	victim->move    /= 2;
	dam              = dice(1, level);
	ch->hit         += dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("You start to draw energy... Wow! what a rush!\n\r",ch);
    damage_old( ch, victim, dam*SPELL_DAM, sn, DAM_NEGATIVE ,TRUE);
    return;
}



void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("Your fireball explodes in a cloud of steam.\n\r", ch);
	return;
    }
    if (ch != victim)    
        send_to_char("You send a raging inferno towards your opponent.\n\r",ch);
    send_to_char("You are blasted apart by a huge ball of flame.\n\r",victim);
    dam         = (ch->fire * 10) + dice(12,10);

    if (is_wearing_artifact(ch, ARTIFACT_ARMET) != NULL)
    {
	dam *= 3;
	send_to_char("Your face is warmed as your helm heats up.\n\r", ch);
	act("$n's helm glows brilliant red.", ch, NULL, NULL, TO_ROOM);
    }

    if ( saves_spell( level, victim, DAM_FIRE) )
	dam /= 2;
    damage_old( ch, victim, dam*SPELL_DAM, sn, DAM_FIRE ,TRUE);
    return;
}

void spell_iceball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (ch != victim)    
    {
	act("You send a freezing blast of cold towards $N.", ch, NULL, victim, TO_CHAR);
        act("$n sends a freezing blast of cold towards $N.", ch, NULL, victim, TO_NOTVICT);
        act("$n sends a freezing blast of cold towards you.", ch, NULL, victim, TO_VICT);
    }

    dam         = (ch->water * 15) + dice(10,10);

    if ( saves_spell( level, victim, DAM_COLD) )
	dam /= 2;
    damage_old( ch, victim, dam*SPELL_DAM, sn, DAM_COLD ,TRUE);
    return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
	act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
	return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}



void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6 + level / 2, 8);
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage_old( ch, victim, dam*SPELL_DAM, sn, DAM_FIRE ,TRUE);
    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	{
	send_to_char("They are already affected.\n\r",ch);
	return;
	}

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_bloodlust( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return;
    }

    if ( IS_AFFECTED(victim, AFF_BLOODLUST) )
    {
	send_to_char("They already lust for blood.\n\r",ch);
	return;
    }
   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 8 + (ch->water * 3);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_BLOODLUST;
    affect_to_char( victim, &af );
    send_to_char( "You have an overwhelming thirst for blood.\n\r", victim );
    act( "$n clutches $s throat and gasps for blood.", victim, NULL, NULL, TO_ROOM );
    return;
}


void spell_mummy_rot( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim, DAM_NEGATIVE) )
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return;
    }

    if (victim->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("They are already dead!\n\r", ch);
	return;
    }

    if ( IS_AFFECTED(victim, AFF_MUMMY_ROT) )
    {
	send_to_char("They already have the disease.\n\r",ch);
	return;
    }
   
    af.where     = TO_FLAG3;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 8 + (ch->entropy * 3);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_MUMMY_ROT;
    affect_to_char( victim, &af );
    send_to_char( "Your skin begins to peel away and rot.\n\r", victim );
    act( "$n's skin begins to peel away and rot.", victim, NULL, NULL,TO_ROOM );
    return;
}

void spell_blood_spew( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim, DAM_NEGATIVE) )
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return;
    }

    if (victim->secondaryrace == RACE_VAMPIRE)
    {
	send_to_char("They have no blood to spew!\n\r", ch);
	return;
    }

    if ( IS_AFFECTED(victim, AFF_BLOOD_SPEW) )
    {
	send_to_char("They already have the disease.\n\r",ch);
	return;
    }
   
    af.where     = TO_FLAG3;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10 + (ch->entropy * 3);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_BLOOD_SPEW;
    affect_to_char( victim, &af );
    send_to_char( "You start to cough up blood.\n\r", victim );
    act( "$n starts to cough up blood.", victim, NULL, NULL,TO_ROOM );
    return;
}

void spell_fear( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return;
    }

    if ( IS_NEWFLAG(victim, AFF_FEAR) )
	{
	send_to_char("They are already running for their lives.\n\r",ch);
	return;
	}

    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    if (ch->entropy > 8) af.duration +=1;
    if (ch->entropy > 9) af.duration +=1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FEAR;
    affect_to_char( victim, &af );
    send_to_char( "You panic as you are gripped by an incredible fear.\n\r", victim );
    act( "$n screams and runs away.", victim, NULL, NULL, TO_ROOM );
    exec_fear(victim);
    return;
}

void spell_deafness( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return;
    }

    if ( IS_NEWFLAG(victim, AFF_FEAR) )
    {
	send_to_char("They are already deaf.\n\r",ch);
	return;
    }

    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->entropy + 4;
    if (ch->entropy > 8) af.duration +=1;
    if (ch->entropy > 9) af.duration +=1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_MAGIC_DEAF;
    affect_to_char( victim, &af );
    send_to_char( "You suddenly cannot hear a thing.\n\r", victim );
    return;
}

void spell_allhallowseve( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;
    CHAR_DATA *victim;

    for ( victim = char_list; victim != NULL; victim = victim->next)
    {
	if  (victim->in_room != NULL && ch->in_room != NULL
	&&  victim->in_room->area == ch->in_room->area
	&&  victim != ch)
	{
	    if ( IS_NEWFLAG(victim, AFF_FEAR) )
		continue;
	    af.where     = TO_NEWFLAGS;
	    af.type      = sn;
	    af.level     = level;
	    af.duration  = 1;
	    if (ch->entropy > 8) af.duration +=1;
	    if (ch->entropy > 9) af.duration +=1;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_FEAR;
	    affect_to_char( victim, &af );
	    send_to_char( "You panic as you are gripped by an unnatural fear.\n\r", victim );
	    act( "$n screams and runs away.", victim, NULL, NULL, TO_ROOM );
	    exec_fear(victim);
	}
    }
    return;
}

void spell_conflaguration( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam; 

    if ( IS_AFFECTED(victim, AFF_CONFLAGURATION) )
	{
	send_to_char("They are already immolated in flames.\n\r",ch);
	return;
	}

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 3;
    af.location  = APPLY_AC;
    af.modifier  = (2 * level) + 20;
    af.bitvector = AFF_CONFLAGURATION;
    dam = dice( (6 + level) / 2 , 8) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_FIRE ,TRUE);
    affect_to_char( victim, &af );
    send_to_char( "You scream as the fires of hell envelop you.\n\r", victim );
    act( "$n is enveloped by the fires of hell.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->altitude != ch->altitude) continue;
	if (ich->invis_level > 0)
	    continue;

	if ( ich == ch || saves_spell( level, ich,DAM_OTHER) )
	    continue;

	affect_strip ( ich, gsn_invis                   );
	affect_strip ( ich, gsn_mass_invis              );
	affect_strip ( ich, gsn_sneak                   );
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE       );
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE  );
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK      );
	REMOVE_BIT   ( ich->flag2, AFF_IMPROVED_INVIS   );
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]      = ch->level * 10; /* 10 pounds per level capacity */
    disc->value[3]      = ch->level * 5; /* 5 pounds per level max per item */
    disc->timer         = ch->level * 2 - number_range(0,level / 2); 

    act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You create a floating disc.\n\r",ch);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE,TRUE);
    return;
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_flying(victim) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_join( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    victim->position = POS_STANDING;
    return;
}

void spell_regrowth( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_FLAG6(ch, AFF6_REGROWTH) )
    {
	if (victim == ch)
	  send_to_char("You are already regenerating at an incredible rate.\n\r",ch);
	else
	  act("$N is already regenerating at an incredible rate.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->lifemagic;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF6_REGROWTH;
    affect_join( victim, &af );
    send_to_char( "You start regenerating at an incredible rate.\n\r", victim );
    act( "$n starts regenerating before your very eyes.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_water_breathing( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_WATERBREATHING) )
    {
	send_to_char("You can already breathe underwater.\n\r",ch);
	return;
    }
    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->air + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_WATERBREATHING;
    affect_to_char( ch, &af );
    send_to_char( "Your neck starts to itch, as small gills break the surface.\n\r", ch);
    return;
}


void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   (victim->earth * 11 >= level + 3)
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }   
    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room, victim->altitude);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room, victim->altitude);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }
}



void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already as strong as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX(  20, victim->hit - dice(1,4) ) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_HARM) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage_old( ch, victim, dam, sn, DAM_HARM ,TRUE);
    return;
}

void spell_quench( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    dam = 0;
    if (victim->fire > 6)
	dam = dice (ch->level + ch->water, 20);
    if (victim->fire == 1 || victim->fire == 2)
    {
	victim->fire -= 1;
	send_to_char("You feel less experienced in the realm of flame.\n\r",victim);
    }
    if (!IS_NPC(victim))
    {
	victim->pcdata->condition[COND_THIRST] = 100;
    }
    dam +=20;
    send_to_char("Your flames are quenched, your lungs fill with the water of Lothariel.\n\r",victim);
    send_to_char("You quench the fire.\n\r",ch);        
    if ( saves_spell( level, victim,DAM_COLD) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_COLD ,TRUE);
    damage_old( ch, ch, 10, sn, DAM_COLD, TRUE);
    return;
}

void spell_horrid_wilting( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = victim->water * victim->water * victim->water * 3;
    if (!IS_NPC(victim))
    {
	victim->pcdata->condition[COND_THIRST] = 0;
    }
    send_to_char("Your skin dries up and you gasp for water.\n\r",victim);      
    send_to_char("You drain the water out of your victim.\n\r",ch);     
    if ( saves_spell( level, victim,DAM_HARM) )
	dam /= 2;
    dam += 20;
    damage_old( ch, victim, dam, sn, DAM_HARM ,TRUE);
    return;
}

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
	act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_improved_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
	act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_FLAG5(victim,AFF5_IMPROVED_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("improved slow")))
	{
	    if (victim != ch)
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel much faster.\n\r",victim);
	    return;
	}
	act("$n is moving much less slowly.",victim,NULL,NULL,TO_ROOM);
	return;
    }

    af.where     = TO_FLAG5;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/3 + dice(1, 10);
    else
      af.duration  = level/6 + dice(1, 10);
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 28) + (level >= 35) + (level >= 62);
    af.bitvector = AFF5_IMPROVED_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving much more quickly.\n\r", victim );
    act("$n is moving much more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
	send_to_char( "You bless them with improved quickness.\n\r", ch );
    return;
}



void spell_spring_of_life( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100 + dice (20,10), victim->max_hit );
    update_pos( victim );
    send_to_char( "The healing waters of Lothariel flood your soul.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Lothariel smiles upon your actions.\n\r", ch );
    if (IS_FLAG3(victim, AFF_MUMMY_ROT) )
    {
	affect_strip(victim, gsn_mummy_rot);
	send_to_char("The rotting disease fades away.\n\r", victim);
    }
    if (IS_FLAG3(victim, AFF_BLOOD_SPEW) )
    {
	affect_strip(victim, gsn_blood_spew);
	send_to_char("You stop spewing blood.\n\r", victim);
    }
    if (IS_FLAG3(victim, AFF_LYCANTHROPY) )
    {
	REMOVE_BIT(victim->flag3, AFF_LYCANTHROPY);
	act("$N is now free of the curse of lycanthropy.", ch, NULL, victim, TO_CHAR);
	send_to_char("You feel the curse has been lifted from you.\n\r", victim);
	victim->move /= 4;
	victim->hit /= 4;
	send_to_char("You feel totally drained.\n\r", victim);
    }

    return;
}

void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	act( "The waters of Lothariel flow from you to $N.",ch,NULL,victim, TO_CHAR);
    return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
	for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
	{
	    obj_next = obj_lose->next_content;
	    if ( number_range(1,2 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
	    {
		switch ( obj_lose->item_type )
		{
		case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) < 
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
			act("$n yelps and throws $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
			act("You remove and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 3);
			obj_from_char(obj_lose);
			obj_to_room(obj_lose, victim->in_room, victim->altitude);
			fail = FALSE;
		    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
			act("$n yelps and throws $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
			act("You and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 6);
			obj_from_char(obj_lose);
			obj_to_room(obj_lose, victim->in_room, victim->altitude);
			fail = FALSE;
		    }
		    else /* cannot drop */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
		    }
		}
		break;
		case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose) 
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
			act("$n is burned by $p, and throws it to the ground.",
			    victim,obj_lose,NULL,TO_ROOM);
			send_to_char(
			    "You throw your red-hot weapon to the ground!\n\r",
			    victim);
			dam += 1;
			obj_from_char(obj_lose);
			obj_to_room(obj_lose,victim->in_room, victim->altitude);
			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
			act("$n throws a burning hot $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
			act("You and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 6);
			obj_from_char(obj_lose);
			obj_to_room(obj_lose, victim->in_room, victim->altitude);
			fail = FALSE;
		    }
		    else /* cannot drop */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
		    }
		}
		break;
		}
	    }
	}
    } 
    if (fail)
    {
	send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3 * SPELL_DAM;
	damage_old(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
	  send_to_char("You feel full more powerful.\n\r",vch);
	  spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
	  spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
	    spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,6);
	    damage_old(ch,vch,dam*SPELL_DAM,sn,DAM_ENERGY,TRUE);
	  }
	}

	else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
	    spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,4);
	    damage_old(ch,vch,dam*SPELL_DAM,sn,DAM_ENERGY,TRUE);
	  }
	}
    }  
    send_to_char("You feel drained.\n\r",ch);
    ch->move /= 2;
}
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    if (obj == NULL)
    {
	stc("The item is unidentifiable.\n\r", ch);
	return;
    }

    sprintf( buf,
	"Object '%s' is type %s, extra flags %s.\n\rWeight is %d pound%s.\n\r",
	(obj->short_descr) == NULL ? "(Null)" : obj->short_descr,
	item_table[obj->item_type].name,
	extra_bit_name( obj->extra_flags ),
	obj->weight / 10,
	(obj->weight/10) == 1 ? "" : "s");
    send_to_char( buf, ch );

    if (obj->material != NULL)
    {
    	sprintf(buf, "It seems to be made of %s.\n\r", obj->material);
    	send_to_char(buf, ch);	
    }

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" '",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("'",ch);
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d charges of level %d",
	    obj->value[2], obj->value[0] );
	send_to_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_DRINK_CON:
	sprintf(buf,"It holds %s-colored %s.\n\r",
	    liq_table[obj->value[2]].liq_color,
	    liq_table[obj->value[2]].liq_name);
	send_to_char(buf,ch);
	break;

    case ITEM_CONTAINER:
	sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	send_to_char(buf,ch);
	if (obj->value[4] != 100)
	{
	    sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;
		
    case ITEM_WEAPON:
	send_to_char("Weapon type is ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);       break;
	    case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);        break;  
	    case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);       break;
	    case(WEAPON_SPEAR)  : send_to_char("spear/staff.\n\r",ch);  break;
	    case(WEAPON_MACE)   : send_to_char("mace/club.\n\r",ch);    break;
	    case(WEAPON_AXE)    : send_to_char("axe.\n\r",ch);          break;
	    case(WEAPON_FLAIL)  : send_to_char("flail.\n\r",ch);        break;
	    case(WEAPON_WHIP)   : send_to_char("whip.\n\r",ch);         break;
	    case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);      break;
	    default             : send_to_char("unknown.\n\r",ch);      break;
	}
	if (obj->pIndexData->new_format)
	    sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf( buf, "Damage is %d to %d (average %d).\n\r",
		obj->value[1], obj->value[2],
		( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char( buf, ch );
	break;

    case ITEM_ARMOR:
	sprintf( buf, 
	"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
	break;
    default : break;
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
	    if (paf->bitvector)
	    {
		switch(paf->where)
		{
		    case TO_AFFECTS:
			sprintf(buf,"Adds %s affect.\n",
			    affect_bit_name(paf->bitvector));
			break;
		    case TO_OBJECT:
			sprintf(buf,"Adds %s object flag.\n",
			    extra_bit_name(paf->bitvector));
			break;
		    case TO_IMMUNE:
			sprintf(buf,"Adds immunity to %s.\n",
			    imm_bit_name(paf->bitvector));
			break;
		    case TO_RESIST:
			sprintf(buf,"Adds resistance to %s.\n\r",
			    imm_bit_name(paf->bitvector));
			break;
		    case TO_VULN:
			sprintf(buf,"Adds vulnerability to %s.\n\r",
			    imm_bit_name(paf->bitvector));
			break;
		    default:
			sprintf(buf,"Unknown bit %d: %d\n\r",
			    paf->where,paf->bitvector);
			break;
		}
		send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
	    if ( paf->duration > -1)
		sprintf(buf,", %d hours.\n\r",paf->duration);
	    else
		sprintf(buf,".\n\r");
	    send_to_char(buf,ch);
	    if (paf->bitvector)
	    {
		switch(paf->where)
		{
		    case TO_AFFECTS:
			sprintf(buf,"Adds %s affect.\n",
			    affect_bit_name(paf->bitvector));
			break;
		    case TO_OBJECT:
			sprintf(buf,"Adds %s object flag.\n",
			    extra_bit_name(paf->bitvector));
			break;
		    case TO_WEAPON:
			sprintf(buf,"Adds %s weapon flags.\n",
			    weapon_bit_name(paf->bitvector));
			break;
		    case TO_IMMUNE:
			sprintf(buf,"Adds immunity to %s.\n",
			    imm_bit_name(paf->bitvector));
			break;
		    case TO_RESIST:
			sprintf(buf,"Adds resistance to %s.\n\r",
			    imm_bit_name(paf->bitvector));
			break;
		    case TO_VULN:
			sprintf(buf,"Adds vulnerability to %s.\n\r",
			    imm_bit_name(paf->bitvector));
			break;
		    default:
			sprintf(buf,"Unknown bit %d: %d\n\r",
			    paf->where,paf->bitvector);
			break;
		}
		send_to_char(buf,ch);
	    }
	}
    }

    /* Dealing with perm spells, i better add a few more */
    if (obj->perm_spell == ITEMA_SANCTUARY)
	stc("This item surrounds its wearer with a holy sanctuary.\n\r", ch);
    else if (obj->perm_spell == ITEMA_HASTE)
	stc("This item grants the wearer speed.\n\r", ch);
    else if (obj->perm_spell == ITEMA_WINDTOMB)
	stc("This item surrounds the wearer in a tomb of wind.\n\r", ch);
    else if (obj->perm_spell == ITEMA_FLY)
	stc("This item grants the wearer the power of flight.\n\r", ch);
    else if (obj->perm_spell == ITEMA_ACCURACY)
	stc("This item makes the wearer more accurate in combat.\n\r", ch);
    else if (obj->perm_spell == ITEMA_DETECT_EVIL)
	stc("This item gives the wearer the power to detect evil.\n\r", ch);
    else if (obj->perm_spell == ITEMA_INVISIBILITY)
	stc("This item veils the wearer under a cloak of invisibility.\n\r", ch);
    else if (obj->perm_spell == ITEMA_DETECT_MAGIC)
	stc("This item confers the ability to detect magic.\n\r", ch);
    else if (obj->perm_spell == ITEMA_ARMOR)
	stc("This item gives a shield of armor to protect the wearer.\n\r", ch);
    else if (obj->perm_spell == ITEMA_STONE_SKIN)
	stc("This item makes the wearer's skin as hard as stone.\n\r", ch);
    else if (obj->perm_spell == ITEMA_SHIELD)
	stc("This item conjures up a magical shield of protection.\n\r", ch);
    else if (obj->perm_spell == ITEMA_DETECT_GOOD)
	stc("This item confers the ability to see the pure of heart.\n\r", ch);
    else if (obj->perm_spell == ITEMA_BLESS)
	stc("This item will continually bless the wearer.\n\r", ch);
    else if (obj->perm_spell == ITEMA_WATER_BREATHING)
	stc("This item grants the ability to breathe underwater.\n\r", ch);
    else if (obj->perm_spell == ITEMA_IRON_SKIN)
	stc("This item will transform the wearers skin into iron.\n\r", ch);
    else if (obj->perm_spell == ITEMA_PLAGUE)
	stc("This item will plague its wearer.\n\r", ch);
    else if (obj->perm_spell == ITEMA_PROTECTION_EVIL)
	stc("This item grants a limited protection from evil.\n\r", ch);

    /* More verbose descriptions for cool weapons!!!! */
    if (obj->item_type == ITEM_WEAPON)
    {
  	if (IS_SET(obj->value[4], WEAPON_FLAMING))
	    stc("This lethal weapon will flame upon command and burn enemies.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_FROST))
	    stc("This powerful weapon will freeze your enemies.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_VAMPIRIC))
	    stc("This evil artifact will drain the life from your foes.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SHARP))
	    stc("This blade is amazingly sharp.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_VORPAL))
	    stc("This blade has the power to cut off limbs.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_TWO_HANDS))
	    stc("This heavy weapon must be held in two hands.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SHOCKING))
	    stc("This weapon will electrify whoever it touches in combat.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_POISON))
	    stc("This blade is dripping with venom.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SUNBLADE))
	    stc("This artifact is bursting with the radiance of the sun.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_DRAGONLANCE))
	    stc("This weapon has been carefully forged to slay dragons.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SILVER))
	    stc("This weapon is made from purest silver.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SLAY_UNDEAD))
	    stc("This weapon has been crafted to smite undead.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_CAUSTIC))
	    stc("This weapon is covered with demonic slime.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SMITE_ELADRIN))
	    stc("This weapon will murder all eladrin it finds.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SMITE_DRAGONS))
	    stc("This weapon will murder all dragons it finds.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SMITE_WARLOCKS))
	    stc("This weapon will kill all warlocks it finds.\n\r", ch);
  	if (IS_SET(obj->value[4], WEAPON_SMITE_DEMONS))
	    stc("This weapon will smite all demons it finds.\n\r", ch);
    }

    if (IS_OBJ_STAT2(obj, ITEM_LOYAL))
 	stc("This item is always loyal to the wearer.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_KNOW_OWNER))
 	stc("This artifact will divulge its owner.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_INDESTRUCTABLE))
 	stc("This artifact is near-indestructible.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_DESIRED))
 	stc("This artifact is desired by many.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_ITEMHIDE))
 	stc("This item is invisible to normal sight.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_STAKE))
 	stc("This weapon can be used to stake vampires.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_DAEMONSEED))
 	stc("This item radiates a soft red pulsing glow.\n\r", ch);
    if (IS_OBJ_STAT2(obj, ITEM_FLYING))
 	stc("This item is totally weightless.\n\r", ch);

    /* Magical rings */
    if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER)
	&& obj->value[5] != 0)
	stcf(ch, "This powerful ring contains the spell '%s', with %d charges left.\n\r",
	    skill_table[obj->value[5]].name, obj->value[6]);

    /* Magical bows */
    if (obj->item_type == ITEM_BOW
	&& obj->value[2] != 0)
	stcf(ch, "This powerful bow contains the spell '%s'.\n\r",
	    skill_table[obj->value[2]].name);

    /* Magical arrows */
    if (obj->item_type == ITEM_ARROW
	&& obj->value[5] != 0)
	stcf(ch, "This magical arrow contains the spell '%s'.\n\r",
	    skill_table[obj->value[5]].name);



    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;  

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	
	af.where        = TO_OBJECT;
	af.type         = sn;
	af.level        = level;
	af.duration     = level + 12;
	af.location     = APPLY_NONE;
	af.modifier     = 0;
	af.bitvector    = ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
	|| IS_NEWFLAG(victim, AFF_IMPROVED_INVIS) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}

void spell_stone_grip( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_NEWFLAG(victim, AFF_NO_DISARM) )
	{
	send_to_char("Their grip is already as hard as stone.\n\r", ch);
	return;
	}

    act( "$n's grip on $s weapon tightens.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = ch->earth * 10;
    af.duration  = ch->earth * 6;
    if (af.duration < 20) af.duration = 20;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_NO_DISARM;
    affect_to_char( victim, &af );
    send_to_char( "Your grip on your weapon tightens.\n\r", victim );
    return;
}

void spell_reflection( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_FLAG3(victim, AFF_REFLECTION) && ch != victim)
    {
	send_to_char("They already have a reflective shield.\n\r", ch);
	return;
    }
    else if(IS_FLAG3(victim, AFF_REFLECTION) )
    {
	send_to_char("You already have that spell on you.\n\r", ch);
	return;
    }


    act( "A reflective shield starts to form around $n", victim, NULL,NULL, TO_ROOM );

    af.where     = TO_FLAG3;
    af.type      = sn;
    af.level     = ch->fire * 10;
    af.duration  = ch->fire * 3;
    if (af.duration < 20) af.duration = 20;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_REFLECTION;
    affect_to_char( victim, &af );
    send_to_char( "A magical reflective shield starts to form around you.\n\r", victim );
    return;
}


void spell_improved_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
 {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
	|| IS_NEWFLAG(victim, AFF_IMPROVED_INVIS) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->air + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_IMPROVED_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}


void spell_pentagram( int sn, int level, CHAR_DATA *ch, void *vo,int target )
 {
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_PENTAGRAM ) )
	return;

    act( "$n inscribes a chalk circle in the ground.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->entropy * 2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PENTAGRAM;
    affect_to_char( victim, &af );
    send_to_char( "You scribe a chalk circle around yourself.\n\r", victim );
    return;
}



void spell_hold_person( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if (is_flying(victim) )
    {
	send_to_char("Your victim must be touching the ground.\n\r", ch);
	return;
    }

    if (saves_spell(level , victim,DAM_COLD))
	{
	send_to_char("The spell fizzles and dies.\n\r", ch);
	return;
	}

    if ( IS_NEWFLAG(victim, AFF_HOLD) )
    {
	send_to_char("They are already frozen.\n\r",ch);
	return;
    }

    act( "$n's legs stiffen as the enchantment takes effect.", victim, NULL, NULL, TO_ROOM );
    send_to_char("Your legs slowly stiffen, you are paralysed.\n\r", victim);
    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_HOLD;
    affect_to_char( victim, &af );
    send_to_char( "Your feet are frozen in place.\n\r", victim );
    return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;
    if (dice(1,ch->earth) == 1)
    {
	ap = 0 - ap;
    }

	 if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    if (dice(1,ch->earth) == 1)
    {
	send_to_char("But you aren't really sure...",ch);
    }

    return;
}



void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,      0,  0,  0, 25, 28,
	31, 34, 37, 40, 40,     41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,     48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,     54, 55, 56, 56, 57,
	58, 58, 59, 60, 60,     61, 62, 62, 63, 64
    };
    int dam;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("Your lightning bolt is conducted by the water around you.\n\r", ch);
	damage_old(ch, ch, 60*SPELL_DAM, sn, DAM_LIGHTNING, TRUE);
	return;
    }
    send_to_char("A bolt of pure energy streaks towards your victim.\n\r",ch);  
    send_to_char("The jagged bolt of energy strikes you in the chest.\n\r",victim);     
    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2 ) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_LIGHTNING) )
	dam /= 2;

    if (is_affected(victim, skill_lookup("reflection"))
	|| victim->power[DISC_DAEM_GELU] > 4 )
    {
	act("Your bolt reflects off $N's shield.", ch,NULL,victim,TO_CHAR);
	act("$n's lightning bolt reflects off $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n's lightning bolt reflects off your shield.",ch,NULL,victim,TO_VICT);
        damage_old( ch, ch, dam, sn, DAM_LIGHTNING ,TRUE);
        return;
    }


    damage_old( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level
	||   ch->level < obj->level)
	    continue;

	found = TRUE;
	number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "one is carried by %s\n\r",
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "one is in %s [Room %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
		sprintf( buf, "one is in %s\n\r",
		    in_obj->in_room == NULL
			? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer,buf);

	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  5,      6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,      8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,     10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,     12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,     14, 14, 14, 14, 14
    };
    int dam;

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2);
    dam *= SPELL_DAM;
    if ( saves_spell( level, victim,DAM_ENERGY) )
	dam /= 2;
    if (IS_FLAG3(victim, AFF_REFLECTION)
	|| victim->power[DISC_DAEM_GELU] > 4 )

    {
	act("Your missile reflects off $N's shield.", ch,NULL,victim,TO_CHAR);
	act("$n's magic missile reflects off $n's shield.", ch,NULL,victim,TO_NOTVICT);
	act("$n's magic missile reflects off your shield.",ch,NULL,victim,TO_VICT);
        damage_old( ch, ch, dam, sn, DAM_ENERGY ,TRUE);
        return;
    }

    damage_old( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
    return;
}

void spell_ballistic_attack( int sn, int level, CHAR_DATA *ch,void *vo,int 
target) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  4,      5,  5,  5,  5,  6,
	 6,  6,  6,  6,  6,      7,  7,  8,  8,  8,
	 9,  9,  9,  9,  9,     10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,     12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,     14, 14, 14, 14, 14
    };
    int dam;

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2 ) * SPELL_DAM;
    if ( saves_spell( level, victim,DAM_ENERGY) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_ENERGY ,TRUE);
    return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (gch->altitude != ch->altitude) continue;
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if (gch->altitude != ch->altitude) continue;
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) 
		|| IS_NEWFLAG(gch, AFF_IMPROVED_INVIS))
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim->secondaryrace == RACE_VAMPIRE 
	|| victim->power[DISC_DAEM_CORR] >= 1)
    {
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	send_to_char("They dont seem to be affected.\n\r", ch);
	return;
    }

    if (saves_spell(level,victim,DAM_DISEASE) || 
	(IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level * 3/4;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 0 - (ch->entropy /2) - 1; 
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}

void spell_phant_force( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (saves_spell(level,victim,DAM_OTHER)) 
    {
	if (ch == victim)
	  send_to_char("Your vision blurs for a second.\n\r",ch);
	else
	  act("$N is not affected by your spell.",ch,NULL,victim,TO_CHAR);
	return;
    }

    rand_to_char
      (victim, "You scream in agony as plague sores erupt from your skin.\n\r",
       "A small gnome taps you on the shoulder.\n\r",
       "A huge red dragon swoops overhead, and lands just to the north.\n\r",
       "You notice a secret door to the west.\n\r",
       "Your purse feels lighter.\n\r");
}

void spell_imp_phant_force( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (saves_spell(level,victim,DAM_OTHER)) 
    {
	if (ch == victim)
	  send_to_char("Your vision blurs for a moment.\n\r",ch);
	else
	  act("$N is not affected by your spell.",ch,NULL,victim,TO_CHAR);
	return;
    }

    rand_to_char
      (victim, "Black tentacles explode from the ground and wrap around your legs.\n\r",
       "A loud voice booms 'FREEZE MORTAL! OR YOU WILL BE SLAIN'\n\r",
       "A guardian vampire appears in a puff of smoke.\n\r",
       "You scream as the fires of hell envelop you.\n\r",
       "Argh! The pain! You scream as your arm is ripped out of its socket.\n\r");
}

void spell_hideous_laughter( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_NEWFLAG(victim, AFF_LAUGHTER) )
    {
	send_to_char("Your victim is already affected.\n\r", ch);
	return;
    }

    if (saves_spell(level,victim,DAM_FIRE))
    {
	if (ch == victim)
	  send_to_char("You feel a bit giggly for a second, then it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_NEWFLAGS;
    af.type       = sn;
    af.level      = level * 3/4;
    af.duration  = ch->air + 4;
    af.location  = APPLY_SAVES;
    af.modifier  = -2; 
    af.bitvector = AFF_LAUGHTER;
    affect_join(victim,&af);
   
    send_to_char
      ("You smile, giggle and then start to roar with fits of laughter.\n\r",victim);
    act("$n suddenly bursts into fits of laughter.",
	victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
	    return;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
	    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	    ||  IS_WEAPON_STAT(obj,WEAPON_DRAGONLANCE)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SUNBLADE)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SILVER) 
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    af.where     = TO_WEAPON;
	    af.type      = sn;
	    af.level     = level / 2;
	    af.duration  = level/8;
	    af.location  = 0;
	    af.modifier  = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim,DAM_POISON)
	|| IS_SET(victim->imm_flags, IMM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    {
	if (victim == ch)
	  send_to_char("You are already protected.\n\r",ch);
	else
	  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
	act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
    return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
    ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
    {
	if (victim == ch)
	  send_to_char("You are already protected.\n\r",ch);
	else
	  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
	act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;
 
    if (IS_EVIL(ch) )
    {
	victim = ch;
	send_to_char("The energy explodes inside you!\n\r",ch);
    }
 
    if (victim != ch)
    {
	act("$n raises $s hand, and a blinding ray of light shoots forth!",
	    ch,NULL,NULL,TO_ROOM);
	send_to_char(
	   "You raise your hand and a blinding ray of light shoots forth!\n\r",
	   ch);
    }

    if (IS_GOOD(victim))
    {
	act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return;
    }

    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000 * SPELL_DAM;

    damage_old( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    spell_blindness(gsn_blindness, 
	3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}

void spell_unravel( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *obj2;
    char buf[MAX_STRING_LENGTH];

    if (obj->item_type != ITEM_ARMOR
	&& !is_name("cloak", obj->name)
	&& !is_name("shirt", obj->name)
	&& !is_name("blouse", obj->name)
	&& !is_name("robe", obj->name)
	&& !is_name("trousers", obj->name)
	&& !is_name("gown", obj->name))
    {
	send_to_char("You can only unravel clothes.\n\r",ch);
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)
	|| IS_OBJ_STAT(obj, ITEM_RELIC))
    {
	send_to_char("That item is too powerful to affect.\n\r",ch);
	return;
    }

    if (str_cmp(obj->material, "cloth")
	&& !is_name("cloak", obj->name)
	&& !is_name("shirt", obj->name)
	&& !is_name("blouse", obj->name)
	&& !is_name("robe", obj->name)
	&& !is_name("trousers", obj->name)
	&& !is_name("gown", obj->name))
    {
	send_to_char("You can only unravel things made of cloth.\n\r",ch);
	return;
    }

    act("$p unravels in your hands until it is merely bits of thread.", ch, obj, NULL, TO_CHAR);
    act("$n lays $s hands on $p and it unravels.", ch, obj, NULL, TO_ROOM);

    if (obj->last_owner == NULL)
    {
        extract_obj(obj);
	return;
    }

    obj2= create_object(get_obj_index(12011), 0);
    obj_to_char(obj2, ch);
    SET_BIT(obj2->extra_flags2, ITEM_VOO_THREAD);

    sprintf(buf, obj2->short_descr, obj->last_owner );
    free_string(obj2->short_descr);
    obj2->short_descr = str_dup(buf);
        
    sprintf(buf, obj2->name, obj->last_owner );
    free_string(obj2->name);
    obj2->name = str_dup(buf);

    free_string(obj2->voodoo_name);
    obj2->voodoo_name = str_dup(obj->last_owner);
    extract_obj(obj);

}

void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("That item has already been recharged once.\n\r",ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_ROOM);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }   

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_char("Nothing seems to happen.\n\r",ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else /* whoops! */
    {
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
    }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
	send_to_char("You feel fully refreshed!\n\r",victim);
    else
	send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL);
		return;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,gsn_curse))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
	if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
	{   /* attempt to remove curse */
	    if (!saves_dispel(level,obj->level,0))
	    {
		found = TRUE;
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
		act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
	    }
	 }
    }
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a shield of energy.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a shield of energy.\n\r", victim );
    return;
}



void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] = 
    {
	 0,
	 0,  0,  0,  0,  0,      0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,     40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,     45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,     50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,     55, 56, 56, 57, 57
    };
    int dam;
    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("Your lightning bolt is conducted by the water around you.\n\r", ch);
	damage_old(ch, ch, 60, sn, DAM_LIGHTNING, TRUE);
	return;
    }

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 4, dam_each[level] * 2 ) * SPELL_DAM;


    if (is_affected(victim, skill_lookup("reflection"))
        || victim->power[DISC_DAEM_GELU] > 4 )
    {
        act("Your shocking grasp reflects off $N's shield.", ch,NULL,victim,TO_CHAR);
        act("$n's shocking grasp reflects off $N.",ch,NULL,victim,TO_NOTVICT);
        act("$n's shocking grasp reflects off your shield.",ch,NULL,victim,TO_VICT);
        if ( saves_spell( level, ch, DAM_LIGHTNING) )
            dam /= 2;
        damage_old( ch, ch, dam, sn, DAM_LIGHTNING ,TRUE);
        return;
    }

    if ( saves_spell( level, victim,DAM_LIGHTNING) )
        dam /= 2;

    damage_old( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE);
    return;
}



void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   (level + 2) < victim->level
    ||   saves_spell( level-4, victim,DAM_CHARM) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
	if (victim == ch)
	  send_to_char("You can't move any slower!\n\r",ch);
	else
	  act("$N can't get any slower than that.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
	    send_to_char("Nothing seemed to happen.\n\r",ch);
	send_to_char("You feel momentarily lethargic.\n\r",victim);
	return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
	if (!check_dispel(level,victim,skill_lookup("haste")))
	{
	    if (victim != ch)
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily slower.\n\r",victim);
	    return;
	}

	act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
	return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_improved_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
	if (victim == ch)
	  send_to_char("You can't move any slower!\n\r",ch);
	else
	  act("$N can't get any slower than that.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
	    send_to_char("Nothing seemed to happen.\n\r",ch);
	send_to_char("You feel momentarily lethargic.\n\r",victim);
	return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
	if (!check_dispel(level,victim,skill_lookup("haste")))
	{
	    if (victim != ch)
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily slower.\n\r",victim);
	    return;
	}

	act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_FLAG5(victim,AFF5_IMPROVED_HASTE))
    {
	if (!check_dispel(level,victim,skill_lookup("improved haste")))
	{
	    if (victim != ch)
		send_to_char("Spell failed.\n\r",ch);
	    send_to_char("Your accelerated movements cease.\n\r",victim);
	    return;
	}

	act("$n is moving much less quickly.",victim,NULL,NULL,TO_ROOM);
	return; 
    } 

    af.where     = TO_FLAG5;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 28) - (level >= 35) - (level >= 52);
    af.bitvector = AFF5_IMPROVED_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in super-slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_iron_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as iron.\n\r",ch); 
	else
	  act("$N is already as hard as iron.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (level / 2) + 6;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin is now as hard as iron.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to iron.\n\r", victim );
    return;
}

void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -30;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return;
}

void spell_dragon_trans(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{           
    stc("The spell failed.\n\r", ch);
    return;
}

void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    send_to_char( "Nothing happens.\n\r", ch );
    return;
}


void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5, victim,DAM_OTHER))))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    pRoomIndex = get_rand_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex, 0 );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void spell_chelchemy( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj;

    if (!str_prefix(target_name, "empty vial"))
    {
	if (enough_money(ch, 100 * CNV_GOL))
 	{
	    stc("You do not have enough gold to create a vial.\n\r", ch);
	    return;
  	}
	give_cash(ch, NULL, 100 * CNV_GOL);
	obj = create_object(get_obj_index(51), ch->level);
	obj_to_char(obj, ch);
	act("The gold reforms in the shape of $p.", ch, obj, NULL, TO_CHAR);
	act("$n forms $p out of gold.", ch, obj, NULL, TO_ROOM);
	return;
    }
    stc("You cannot create that item using chelchemy.\n\r", ch);
    return;
}

void spell_whispers( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    target_name = one_argument( target_name, arg );

    if ( (victim = get_char_world(ch, arg)) == NULL)
    {
        stc("You cannot locate that character.\n\r", ch);
        return;
    }

    if (ch == victim)
    {
        stc("You cannot send the message to yourself.\n\r", ch);
        return;
    }
    
    stc("You send the message onto the wind.\n\r", ch);
    sprintf(buf, "You hear the wind whispering '%s'\n\r", target_name);
    stc(buf, victim);
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (vch->altitude != ch->altitude) continue;
	if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
	    send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
    }

    return;
}



void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim,DAM_OTHER) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;
    
    if (IS_NPC(victim))
      return;
   
    if ((location = get_room_index( prime_realm, ROOM_VNUM_START)) == NULL)
    {
	send_to_char("You are completely lost.\n\r",victim);
	return;
    } 

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) ||
	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return;
    }

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);
    
    ch->move /= 2;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,location,0);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_look(victim,"auto");
}

void fire_breath(CHAR_DATA *ch, CHAR_DATA *victim, bool all)
{
    CHAR_DATA *vch, *vch_next;
    int dam;
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("You breathe out a cloud of harmless steam.\n\r", ch);
	act("$n breathes forth a cone of harmless steam.",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a cone of harmless steam over you.",ch,NULL,victim,TO_VICT);
	return;
    }

    if (all)
    {
	dam = (ch->generation +5 * ch->power[DISC_DRAG_FIRE]+5) * 12;
	
        act("$n breathes forth a cone of fire over you.",ch,NULL,NULL,TO_ROOM);
        act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
	    vch_next = vch->next_in_room;

	if (vch->altitude != ch->altitude) continue;

	    if (is_safe_spell(ch,vch,TRUE) 
	    &&   (ch->fighting != vch || vch->fighting != ch))
	        continue;

	    if (vch == ch) continue;

	    if (saves_spell(ch->level,vch,DAM_FIRE))
	    {
	        fire_effect(vch,ch->level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,skill_lookup("fire breath"),DAM_FIRE,TRUE);
		set_fighting(ch, vch);
	    }
	    else
	    {
		fire_effect(vch,ch->level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,skill_lookup("fire breath"),DAM_FIRE,TRUE);
		set_fighting(ch, vch);
	    }
    if (vch->hit < -9 && !IS_IMMORTAL(vch) && vch->secondaryrace != 0)
    {
        send_to_char("You disintegrate into a pile of ashes.\n\r", vch);
        act("$n disintegrates into a pile of ashes.", vch, NULL, NULL, TO_ROOM);
        obj = create_object( get_obj_index(67), 0);
        obj_to_room(obj, vch->in_room, vch->altitude);
        destroy_char(vch, vch, FALSE);
        if (player_kill(ch, vch)) player_kill_update(ch, vch, FALSE);
        sprintf(buf, "--> %s has been disintegrated by %s.",vch->name, ch->name);
        informational(buf, vch->race, 0);
        informational(buf, ch->race, 0);
        return;
    }

	}
	return;
    }

    if (is_safe(ch, victim)) return;
    dam = (ch->generation +3 * ch->power[DISC_DRAG_FIRE]+3) * 20;

    act("$n breathes forth a cone of fire over $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire over $N.",ch,NULL,victim,TO_CHAR);

    fire_effect(victim->in_room,ch->level,dam/2,TARGET_ROOM);
    damage_old(ch,victim,dam,skill_lookup("fire breath"),DAM_FIRE,TRUE);

    if (victim->hit < -9 && !IS_IMMORTAL(victim) && victim->secondaryrace != 0)
    {
        send_to_char("You disintegrate into a pile of ashes.\n\r", victim);
        act("$n disintegrates into a pile of ashes.", victim, NULL, NULL, TO_ROOM);
        obj = create_object( get_obj_index(67), 0);
        obj_to_room(obj, victim->in_room, victim->altitude);
        destroy_char(victim, victim, FALSE);
        if (player_kill(ch, victim)) player_kill_update(ch, victim, FALSE);
        sprintf(buf, "--> %s has been disintegrated by %s.",victim->name, ch->name);
        informational(buf, victim->race, 0);
        informational(buf, ch->race, 0);
    }
}

void acid_breath(CHAR_DATA *ch, CHAR_DATA *victim, bool all)
{
    CHAR_DATA *vch, *vch_next;
    int dam;

    if (all)
    {
	dam = (ch->generation +5 * ch->power[DISC_DRAG_FIRE]+5) * 4;
	
        act("$n breathes forth a stream of acid over you.",ch,NULL,NULL,TO_ROOM);
        act("You breath forth a stream of acid.",ch,NULL,NULL,TO_CHAR);

        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
	    vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;

	    if (is_safe_spell(ch,vch,TRUE) 
	    &&   (ch->fighting != vch || vch->fighting != ch))
	        continue;

	    if (vch == ch) continue;

	    if (saves_spell(ch->level,vch,DAM_ACID))
	    {
	        acid_effect(vch,ch->level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,skill_lookup("acid breath"),DAM_ACID,TRUE);
		set_fighting(ch, vch);
	    }
	    else
	    {
		fire_effect(vch,ch->level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,skill_lookup("acid breath"),DAM_ACID,TRUE);
		set_fighting(ch, vch);
	    }
	}
	return;
    }

    if (is_safe(ch, victim)) return;
    dam = (ch->generation +3 * ch->power[DISC_DRAG_FIRE]+3) * 16;

    act("$n breathes forth a stream of acid over $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a stream of acid over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a stream of acid over $N.",ch,NULL,victim,TO_CHAR);

    acid_effect(victim->in_room,ch->level,dam/2,TARGET_ROOM);
    damage_old(ch,victim,dam,skill_lookup("acid breath"),DAM_ACID,TRUE);
}

void lightning_breath(CHAR_DATA *ch, CHAR_DATA *victim, bool all)
{
    CHAR_DATA *vch, *vch_next;
    int dam;
    all = FALSE;

    dam = (ch->generation +3 * ch->power[DISC_DRAG_FIRE]+3) * 14;

    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("The lightning explodes in a huge blast.\n\r", ch);
	act("$n breathes a bolt of lightning, which is conducted by the water.",ch,NULL,NULL,TO_ROOM);
        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
	    vch_next = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;

	    if (is_safe_spell(ch,vch,TRUE))
	        continue;

	    shock_effect(vch,ch->level,dam,TARGET_CHAR);
	    damage_old(ch,vch,dam,skill_lookup("lightning breath"),DAM_LIGHTNING,TRUE);
	}
	return;
    }

    if (is_safe(ch, victim)) return;
    if (saves_spell(ch->level,victim,DAM_COLD)) dam /= 2;

    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    shock_effect(victim->in_room,ch->level,dam/2,TARGET_ROOM);
    damage_old(ch,victim,dam,skill_lookup("lightning breath"),DAM_LIGHTNING,TRUE);
    set_fighting(ch, victim);
}

void frost_breath(CHAR_DATA *ch, CHAR_DATA *victim, bool all)
{
    CHAR_DATA *vch, *vch_next;
    int dam;
    int power;

    if (ch->secondaryrace == RACE_DRAGON) power = ch->power[DISC_DRAG_FIRE];
    else if (ch->secondaryrace == RACE_DAEMON) power = ch->power[DISC_DAEM_GELU];
    else power = 0;

    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("You freeze the water around you.\n\r", ch);
	act("$n freezes the water around $m slightly.",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (all)
    {
	dam = (ch->generation +5 * power+5) * 8;
	
        act("$n breathes forth a stream of frost over you.",ch,NULL,NULL,TO_ROOM);
        act("You breath forth a stream of frost.",ch,NULL,NULL,TO_CHAR);

        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
	    vch_next = vch->next_in_room;
	    if (vch == ch) continue;
	if (vch->altitude != ch->altitude) continue;

	    if (is_safe_spell(ch,vch,TRUE) 
	    &&   (ch->fighting != vch || vch->fighting != ch))
	        continue;

	    if (saves_spell(ch->level,vch,DAM_COLD))
	    {
	        cold_effect(vch,ch->level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,skill_lookup("frost breath"),DAM_COLD,TRUE);
	        set_fighting(ch, vch);
	    }
	    else
	    {
		cold_effect(vch,ch->level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,skill_lookup("frost breath"),DAM_COLD,TRUE);
	        set_fighting(ch, vch);
	    }
	}
	return;
    }

    if (is_safe(ch, victim)) return;
    dam = (ch->generation +3 * power+3) * 20;

    act("$n breathes forth a stream of frost over $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a stream of frost over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a stream of frost over $N.",ch,NULL,victim,TO_CHAR);

    cold_effect(victim->in_room,ch->level,dam/2,TARGET_ROOM);
    damage_old(ch,victim,dam,skill_lookup("frost breath"),DAM_COLD,TRUE);
    set_fighting(ch, victim);
}

void gas_breath(CHAR_DATA *ch, CHAR_DATA *victim, bool all)
{
    CHAR_DATA *vch, *vch_next;
    int dam;
    all = TRUE;

    if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
	send_to_char("The poison dissolves with the water and washes away.\n\r", ch);
	act("$n breathes a cloud of gas, which is absorbed by the water.",ch,NULL,NULL,TO_ROOM);
	return;
    }

	dam = (ch->generation +5 * ch->power[DISC_DRAG_FIRE]+5) * 4;
	
        act("$n breathes forth a cloud of gas over you.",ch,NULL,NULL,TO_ROOM);
        act("You breath forth a cloud of gas.",ch,NULL,NULL,TO_CHAR);

        for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
        {
	    vch_next = vch->next_in_room;
	    if (vch == ch) continue;
	if (vch->altitude != ch->altitude) continue;

	    if (is_safe_spell(ch,vch,TRUE) 
	    &&   (ch->fighting != vch || vch->fighting != ch))
	        continue;

	    if (saves_spell(ch->level,vch,DAM_POISON))
	    {
	        poison_effect(vch,ch->level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,skill_lookup("gas breath"),DAM_POISON,TRUE);
	    }
	    else
	    {
		poison_effect(vch,ch->level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,skill_lookup("gas breath"),DAM_POISON,TRUE);
	    }
	}
	return;
}

void spell_soul_blade( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
    OBJ_DATA *obj;
    char name[MAX_STRING_LENGTH];

    sprintf(name, "%s's soul blade", ch->name);
    obj         = create_object( get_obj_index(27), 0);
    obj_to_char( obj, ch );
    obj = get_obj_world(ch, "soul blade");
    WRITE_STR(obj->name, name);
    WRITE_STR(obj->short_descr, name);
    send_to_char("You create a shimmering blade of pure energy.\n\r",ch);
    act("$n creates a shimmering blade of pure energy.",ch,NULL,NULL,TO_ROOM);
    obj->timer = number_range(ch->earth*5, ch->earth*6);
}


void spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
    OBJ_DATA *obj;
    char name[MAX_STRING_LENGTH];
    sprintf(name, "%s's flame blade", ch->name);
    obj         = create_object( get_obj_index(30), 0 );
    send_to_char("You create a flaming blade from the fires of hell.\n\r",ch);
    act("$n creates a flaming blade from the fires of hell.",ch,NULL,NULL,TO_ROOM); 
    obj->timer = number_range(ch->fire*3, ch->fire*4);
    obj_to_char(obj, ch);
    obj = get_obj_world(ch, "flame blade");
    WRITE_STR(obj->name, name);
    WRITE_STR(obj->short_descr, name);

}


void spell_shadow_blade( int sn, int level, CHAR_DATA *ch, void *vo,int target ) 
{
    OBJ_DATA *obj;
    char name[MAX_STRING_LENGTH];

    sprintf(name, "%s's shadow blade", ch->name);
    obj         = create_object( get_obj_index(28), 0 );
    obj_to_char( obj, ch );
    obj = get_obj_world(ch, "shadow blade");
    WRITE_STR(obj->name, name);
    WRITE_STR(obj->short_descr, name);
    send_to_char("You create a shimmering blade of pure darkness.\n\r",ch);
    act("$n creates a shimmering blade of pure darkness.",ch,NULL,NULL,TO_ROOM); 
    obj->timer = number_range(ch->entropy*3, ch->entropy*4);
}

void shadow_breath(CHAR_DATA *ch, CHAR_DATA *victim, bool all )
{

}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
	dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_disintegrate( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj = NULL;

    act("A thin green ray shoots out of your finger towards $N.", ch, NULL, victim, TO_CHAR);
    act("A thin green ray shoouts out of $n's finger towards you.", ch, NULL, victim, TO_VICT);
    act("A thin green ray shoots out of $n's finger towards $N.", ch, NULL, victim, TO_NOTVICT);

    if (victim->cur_form == FRM_MIST)
    {
	send_to_char("It passes straight through you.\n\r", victim);
	act("   ...it passes straight through $M", victim, NULL, NULL, TO_ROOM);
	return;
    }

    if (victim->fire > ch->fire || IS_IMMORTAL(victim) )
    {
	send_to_char("The spell didnt work... damn!\n\r", ch);
	return;
    }

    if (IS_FLAG3(victim, AFF_REFLECTION)
	|| victim->power[DISC_DAEM_GELU] > 4 )
    {
	act("Your disintegration ray is reflected from $N's shield.", ch,NULL, victim, TO_CHAR);
	send_to_char("It hits you....  you disintegrate...\n\r",ch);
	act("   ...it bounces back at $n", ch, NULL, NULL, TO_ROOM);
	act("$n disintegrates into a pile of fine powder.", ch, NULL, NULL, TO_ROOM);
	ch->hit = -10;
        obj = create_object( get_obj_index(67), 0);
        obj_to_room(obj, ch->in_room, ch->altitude);
        destroy_char(victim, ch, FALSE);
	if (player_kill(victim, ch) ) player_kill_update(victim, ch, FALSE);
	return;
    }
    else
    {
	act("$n disintegrates into a pile of fine powder.", victim, NULL, NULL, TO_ROOM);
	send_to_char("   ...it hits you, you disintegrate.\n\r", victim);
        victim->hit = -10;
        obj = create_object( get_obj_index(67), 0);
        obj_to_room(obj, victim->in_room, victim->altitude);
        destroy_char(ch, victim, FALSE);
	if (player_kill(ch, victim) ) player_kill_update(ch, victim, FALSE);
        return;
    }

}



void spell_power_word_kill( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	
    if ( (victim->level) >= ch->level )
    {
	send_to_char("The spell failed.\n\r",ch);
	send_to_char("Your heart lurches in your chest, but the feeling passes.\n\r",victim);
	return;
    }
    
    if ( dice (1,100) == 66 )
    {
	send_to_char("The spell backfires!\n\r",ch);
	send_to_char("Your heart lurches in your chest.\n\r",ch);
	damage_old(ch,ch,3000,sn,DAM_NEGATIVE,TRUE);
	return;
    }
    

    ch->alignment = -1000;
    victim->hit = 0;
    send_to_char("You raise your finger, pointing at your victim, and say DIE!\n\r",ch);
    act("$N raises his finger to point at you, and says DIE!.",victim,NULL,ch,TO_CHAR);
    act("$N raises his finger to point at $n, and says DIE!.",victim,NULL,ch,TO_NOTVICT);

    if (can_age(ch))
    {
	stc("You have aged five years.\n\r", ch);
	if (!IS_NPC(ch)) ch->pcdata->extra_age += 5;
    }

    if (victim->level >= 20
	&& victim->secondaryrace == 0
	&& !IS_NPC(victim)
	&& 1 == 0 /* No death knights for now! */
	&& victim->alignment < -666)
    {
	stc("The flesh falls from your bones.\n\r", victim);
	victim->secondaryrace = RACE_DEATH_KNIGHT;
	set_learnable_disciplines(victim, RACE_DEATH_KNIGHT);
	victim->tertiaryrace = 0;
	SET_BIT(victim->act, ACT_UNDEAD);
	victim->generation = 1;
	stc("You have been transformed into a Death Knight.\n\r", victim);
	act("The flesh falls from $n's bones.", victim, NULL, NULL, TO_ROOM);
	set_form(victim, 0);
	victim->clan = CLAN_DAEM_OUTCAST;
	victim->pcdata->damcap = 100;
	return;
    }

    damage_old(ch, victim, 10000 ,sn ,DAM_NEGATIVE ,TRUE);
    return;
}


void spell_wish( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf( buf, "$n wishes: %s", target_name );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character->level > 111)
		act_new(buf,ch,NULL,d->character,TO_VICT,POS_DEAD);
	}
    } 

    return;
}

void spell_consultation( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf( buf, "$n consults: %s", target_name );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character->level > 111)
		act_new(buf,ch,NULL,d->character,TO_VICT,POS_DEAD);
	}
    } 

    return;
}

void spell_arachnophobia( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    send_to_char("Streams of blackened fire shoot up into the heavens.\n\r", ch);
    send_to_char("The sky darkens and maniacal laughter resonates through the air.\n\r", ch);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character->level < 112 && d->character != ch)
		{
		send_to_char("There is a collosal BOOM in the distance.\n\r", d->character);
		sprintf( buf, "You turn into a hairy little spider!");
		act_new(buf,ch,NULL,d->character,TO_VICT,POS_RESTING);
		set_form(d->character, 6);
		}
	}
    } 

    return;
}

void spell_winds_caina( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    DESCRIPTOR_DATA *d;
    int dam;

    dam = dice(ch->air*10,20);

    send_to_char("You summon forth the collosal winds of Caina.\n\r",ch);
    send_to_char("Everything in their path is destroyed.\n\r",ch);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character->level < 112 && d->character != ch)
		{
		send_to_char("The frigid winds of Caina sweep across the land, causing untold devastation.\n\r", d->character);
		send_to_char("The winds sweep you off your feet, and fling you to the ground.\n\r",d->character);
		d->character->hit -= dam;
		if (d->character->hit < 1)
		    d->character->hit = 1;
		}
	}
    } 

    return;
}

void spell_collapse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    DESCRIPTOR_DATA *d;

    send_to_char("There is a sickening thud around you, as the plane of shadows vanishes.\n\r",ch);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && d->character != ch)
	{
	send_to_char("The shadows around you start to vanish...\n\r", d->character);
	    if (d->character->level < 112 && d->character != ch && IS_NEWFLAG(d->character, AFF_SHADOW_PLANE) )
		{
		send_to_char("With an almighty explosion, the shadow plane collapses, with you in it.\n\r", d->character);
		send_to_char("You lose conciousness...\n\r",d->character);
		REMOVE_BIT(d->character->flag2, AFF_SHADOW_PLANE);
		d->character->hit = -10;
		}
	}
    } 
    return;
}

void spell_rock_to_mud( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    send_to_char("You transmute the rocks around you into a sticky, thick mud.\n\r",ch);
    act("$n transmutes the rocks around $m into a morass of sticky mud.",ch,NULL,NULL,TO_ROOM);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character != ch
		&& IS_NEWFLAG(d->character,AFF_EARTH_MELD)
		&& d->character->value[1]==ch->in_room->vnum)
	    {
		REMOVE_BIT(d->character->flag2, AFF_EARTH_MELD);
		sprintf(buf, "%d", d->character->value[1]);
		char_from_room( d->character );
		char_to_room( d->character, find_location( d->character, buf ),0 );
		act("$N screams in pain as their skin is melted", d->character, NULL,NULL,TO_CHAR);
		send_to_char("You scream in agony as your flesh is melted.\n\r",d->character);
		d->character->hit = -10;
		d->character->position = POS_DEAD;
	    }
	    else
	    {
		send_to_char("Your feet are bogged down in the mud.\n\r",d->character);
		d->character->move -= 50;
	    }
	}
    } 

    return;
}

void spell_harmony( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    DESCRIPTOR_DATA *d;

    send_to_char("A wave of harmony passes over the land.\n\r",ch);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character != ch &&
	       d->character->secondaryrace == ch->secondaryrace)
		{
		send_to_char("A golden wave of harmony passes throughout the entire land.\n\r", d->character);
		d->character->hit = d->character->max_hit;
		d->character->move = d->character->max_move;
		affect_strip(d->character, gsn_curse);
		affect_strip(d->character, gsn_plague);
		affect_strip(d->character, gsn_blindness);
		affect_strip(d->character, gsn_conflaguration);
		affect_strip(d->character, gsn_sleep);
		affect_strip(d->character, gsn_poison);
		affect_strip(d->character, gsn_holdperson);
		update_pos(d->character);
		}
	}
    } 
    return;
}

void spell_time_twister( int sn, int level, CHAR_DATA *ch, void *vo,int target) 
{
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    send_to_char("Your mind spins in a vortex of unintelligible screams.\n\r",ch);

    if (!IS_AFFECTED(ch,AFF_HASTE))
    {
	af.where = TO_AFFECTS;
	af.type  = sn;
	af.level = ch->air * 10;
	af.duration = ch->air * 5;
	af.location = APPLY_DEX;
	af.modifier = 2;
	af.bitvector = AFF_HASTE;
	affect_to_char(ch, &af);
    }
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (d->character != ch)
		{
		send_to_char("Time starts flowing in all different directions at once.\n\r", d->character);
		if (d->character->level == 120)
			continue;
		WAIT_STATE(d->character, 5 * PULSE_PER_SECOND);
    if (!IS_AFFECTED(d->character,AFF_SLOW))
	{
	af.where = TO_AFFECTS;
	af.type  = sn;
	af.level = ch->air * 10;
	af.duration = ch->air * 5;
	af.location = APPLY_DEX;
	af.modifier = - ch->air;
	af.bitvector = AFF_SLOW;
	affect_to_char(d->character, &af);
	}
		}
	}
    } 
    return;
}

void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    if (IS_AFFECTED(ch,AFF_BLIND))
    {
        send_to_char("Maybe it would help if you could see?\n\r",ch);
        return;
    }
 
    do_scan(ch,target_name);
}


void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

        if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||	(is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

    stone = get_obj_by_type(ch,ITEM_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room, ch->altitude);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *ob,*obnext;
    CHAR_DATA *victim;

    if (obj->item_type != ITEM_CORPSE_PC && obj->item_type != ITEM_CORPSE_NPC)
    {
	send_to_char("You can only cast this spell on a corpse.\n\r",ch);
	return;
    }

    victim = create_mobile( get_mob_index( 28 ) );
    victim->level = obj->level;
    SET_BIT(victim->flag4, AFF_NO_EXP);

    victim->size = obj->size;
    victim->max_hit = 100 + victim->level * victim->level;
    victim->hit = 100 + victim->level * victim->level;
    char_to_room(victim, ch->in_room, ch->altitude);
    /* Message to room */
	act("$p starts to shudder, and then, slowly, stand up onto its feet.",ch,obj,NULL,TO_ALL);
    /* Take items */
	for (ob = obj->contains; ob != NULL; ob = obnext)
	{
	    obnext = ob->next_content;
	    get_obj( victim, ob, obj );
	}
	do_wear(victim, "all");
    /* charm it */
    add_follower( victim, ch );
    SET_BIT(victim->affected_by, AFF_CHARM);
    victim->leader = ch;
    extract_obj( obj );
}

void spell_raise_dead( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *victim;

    if (obj->item_type != ITEM_CORPSE_PC && obj->item_type != ITEM_CORPSE_NPC)
    {
	send_to_char("You can only cast this spell on a corpse.\n\r",ch);
	return;
    }

    if (obj->item_type == ITEM_CORPSE_PC)
    {
	stc("The life force has already moved.\n\r", ch);
	return;
    }

    if (obj->corpse == NULL)
    {
	stc("You cannot locate any life force within the corpse.\n\r",ch);
	return;
    }
    victim = create_mobile( obj->corpse );
    victim->hit = 1;
    SET_BIT(victim->flag4, AFF_NO_EXP);
    char_to_room(victim, ch->in_room, ch->altitude);
    /* Message to room */
    act("$p starts to shudder, and then, slowly, stand up onto its feet.",ch,obj,NULL,TO_ALL);
    act("$n starts to cough and splutter.", victim, NULL, NULL, TO_ROOM);
    extract_obj( obj );
}

