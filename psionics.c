/******************************************************************
 * Psionics module (C) 1996, by Christopher Mellish               *
 * Psionic skills coded by Christopher Mellish and AdAm PoWeLl    *
 ******************************************************************/


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
#include "psionics.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_psiskill      );
DECLARE_DO_FUN(do_get_obj_world );
DECLARE_DO_FUN(do_psilearn	);

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void    exec_fear       args( ( CHAR_DATA *ch ) );
int	get_psi_disc	args( ( char *argument ) );
int	psi_prime_disc	args( ( CHAR_DATA *ch ) );
char * 	psi_disc_name	args( ( int disc ) );

/* Psionics Handler Functions */

void psi_gain_gen( CHAR_DATA *ch )
{
    int gain;

    if (ch->pcdata == NULL) return;

    if (ch->generation == 0)
    {
	ch->pcdata->psi_disc_points = 0;
	ch->pcdata->psi_def_points  = 0;
	ch->pcdata->psi_sci_points  = 0;
	ch->pcdata->psi_dev_points  = 0;
    }

    ch->generation += 1;

    if (ch->generation < 6)
    {
	stc("You may now learn another psionic defence.\n\r", ch);
	ch->pcdata->psi_def_points += 1;
    }

    if (ch->generation == 1
    ||  ch->generation == 2
    ||  ch->generation == 3
    ||  ch->generation == 5
    ||  ch->generation == 7
    ||  ch->generation == 9
    ||  ch->generation == 11)
    {
	stc("You may now learn another psionic discipline.\n\r", ch);
	ch->pcdata->psi_disc_points += 1;
    }

    stc("You may now learn another psionic science.\n\r", ch);
    ch->pcdata->psi_sci_points += 1;

    gain = 0;
    if (ch->generation == 1) gain = 5;
    else if (ch->generation == 2) gain = 4;
    else gain = 2;

    stcf(ch, "You may now learn %d more psionic devotions.\n\r", gain);
    ch->pcdata->psi_dev_points += gain;
}


void do_psilearn( CHAR_DATA *ch, char *argument )
{
    char arg[1000];

    if (IS_NPC(ch) || ch->secondaryrace != RACE_PSIONICIST)
    {
	stc("You are not a psionicist, so how do you expect to learn psionic powers?\n\r", ch);
	return;
    }

    if (argument[0] == 0)
    {
	char buf[300];

	banner2_to_char("Psionic Learning", ch);
	sprintf(buf, "You can gain %d new discipline%s.",
		ch->pcdata->psi_disc_points,
		(ch->pcdata->psi_disc_points == 1) ? "" : "s");
	cent_to_char(buf, ch);
	sprintf(buf, "You can gain %d new science%s.",
		ch->pcdata->psi_sci_points,
		(ch->pcdata->psi_sci_points == 1) ? "" : "s");
	cent_to_char(buf, ch);
	sprintf(buf, "You can gain %d new devotion%s.",
		ch->pcdata->psi_dev_points,
		(ch->pcdata->psi_dev_points == 1) ? "" : "s");
	cent_to_char(buf, ch);
	sprintf(buf, "You can gain %d new defense%s.",
		ch->pcdata->psi_def_points,
		(ch->pcdata->psi_def_points == 1) ? "" : "s");
	cent_to_char(buf, ch);
	banner2_to_char("Psionic Learning", ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (!str_prefix(arg, "discipline"))
    {
	int disc;

	if (ch->pcdata->psi_disc_points < 1)
	{
	    stc("You cannot learn any more psionic disciplines yet.\n\r", ch);
	    return;
	}

	if (argument[0] == 0)
	{
	    stc("PsiLearn which discipline.\n\r", ch);
	    return;
	}

	disc = get_psi_disc( argument );

	if (disc == -1 || psi_disc_name(disc) == NULL)
	{
	    stc("That is not a psionic discipline.\n\r", ch);
	    return;
	}

	if (psi_prime_disc(ch) < 0)
	{
	    stcf(ch, "You learn %s, making it your prime discipline.\n\r", psi_disc_name(disc));
	    ch->pcdata->psi_disc[disc] = 1;
	    ch->pcdata->psi_disc_points -= 1;
	}
	else
	{
	    stcf(ch, "You learn the discipline of %s.\n\r", psi_disc_name(disc));
	    ch->pcdata->psi_disc[disc] = 2;
	    ch->pcdata->psi_disc_points -= 1;
	}

	return;
    }
}

int psi_prime_disc( CHAR_DATA *ch)
{
    int i;

    for (i = 1; i < 8; i++)
    {
	if (ch->pcdata->psi_disc[i] == 1)
	    return i;
    }

    return -1;
}

char * psi_disc_name( int disc )
{
    if (disc == PSI_PSYCHOMETABOLISM)
	return "psychometabolism";

    if (disc == PSI_PSYCHOKINESIS)
	return "psychokinesis";

    if (disc == PSI_PSYCHOPORTATION)
	return "psychoportation";

    if (disc == PSI_TELEPATHY)
	return "telepathy";

    if (disc == PSI_METAPSIONICS)
	return "metapsionics";

    if (disc == PSI_CLAIRSENTIENCE)
	return "clairsentince";

    if (disc == PSI_ANCIENTS)
	return "ancients";

    return NULL;
}

int get_psi_disc( char *argument )
{
    if (!str_prefix(argument, "psychometabolism"))
	return PSI_PSYCHOMETABOLISM;

    if (!str_prefix(argument, "psychokinesis"))
	return PSI_PSYCHOKINESIS;

    if (!str_prefix(argument, "psychoportation"))
	return PSI_PSYCHOPORTATION;

    if (!str_prefix(argument, "telepathy"))
	return PSI_TELEPATHY;

    if (!str_prefix(argument, "metapsionics"))
	return PSI_METAPSIONICS;

    if (!str_prefix(argument, "clairsentience"))
	return PSI_CLAIRSENTIENCE;

    if (!str_prefix(argument, "ancients"))
	return PSI_ANCIENTS;

    return -1;
}


/* get slot number for a psionic power */
int psi_lookup( const char *name )
{
    int sn;

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++ )
    {
	if ( psi_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(psi_table[sn].name[0])
	&& !str_prefix( name, psi_table[sn].name ) )
	    return sn;
    }

    return -1;
}

/* charging psp's - the global function */

bool charge_psp(CHAR_DATA *ch, int c)
{
    if (ch->level == 120)
	return TRUE;
    if (ch->psp < c)
	return FALSE;
    ch->psp -= c;
    return TRUE;
}

/* find a psionic power and whether the character can use it */
int find_psipower( CHAR_DATA *ch, const char *name )
{
    int sn, found = -1;

    if (IS_NPC(ch))
	return psi_lookup(name);

    for ( sn = 0; sn < NUM_PSI_SLOTS; sn++ )
    {
	if (psi_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(psi_table[sn].name[0])
	&&  !str_prefix(name, psi_table[sn].name))
	{
	    if (found == -1)
		found = sn;
	    if (ch->psi_learnt[sn] != 0)
		return sn;
	}
    }
    return found;
}

/* Add a power to your current Upkeep */

void psi_addupkeep(CHAR_DATA *ch, CHAR_DATA *victim, int sn)
{
    ch->psi_upkeep[sn].victim = victim;
    ch->psi_upkeep[sn].dur = 0;
}

void psi_dropupkeep(CHAR_DATA *ch, int sn)
{
    ch->psi_upkeep[sn].victim = NULL;
    ch->psi_upkeep[sn].dur = -1;
}

void psi_drop_power(CHAR_DATA *ch, int sn)
{
    (*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim,TARGET_CHAR, sn, -1);
    psi_dropupkeep(ch, sn);
}

bool can_use_psionics (CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return FALSE;

    if (ch->level == 120) return TRUE;

    if (is_carrying_artifact(ch, ARTIFACT_STAFF_WIZARDRY) != NULL)
	return FALSE;

    if (is_carrying_artifact(ch, ARTIFACT_CROWN_OF_AGES) != NULL)
	return TRUE;

    if (ch->tertiaryrace != 0) return TRUE;

    if (ch->secondaryrace == RACE_VAMPIRE
	|| ch->secondaryrace == RACE_WARLOCK
	|| ch->secondaryrace == RACE_LUPINE) return FALSE;

    return TRUE;
}




/* Terminate a powers upkeep */

void do_psioff(CHAR_DATA *ch, char *argument)
{
    int sn;

    if (!can_use_psionics(ch) )
    {
	send_to_char("You cannot use psionics.\n\r", ch);
	return;
    }

    if (argument[0]=='\0')
    {
	send_to_char("Stop using which psionic power?\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"all"))
    {
	send_to_char("You stop using all psionic abilities.\n\r",ch);
	for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
	{
	if (ch->psi_upkeep[sn].dur != -1)
	{
    (*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim, TARGET_CHAR,sn, -1);
    psi_dropupkeep(ch, sn);
	}
	}
	return;
    }

    sn = psi_lookup(argument);

    if (sn == -1)
    {
	send_to_char("You do not know any powers of that name.\n\r",ch);
	return;
    }

    if (ch->psi_upkeep[sn].dur == -1)
    {
	send_to_char("You are not currently using that ability.\n\r",ch);
	return;
    }

    (*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim, TARGET_CHAR, sn, -1);
    psi_dropupkeep(ch, sn);
}


/* View Upkeeps */

void do_psiupkeep(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int sn, total = 0, psp = ch->psp;
    bool found=FALSE;

    if (!can_use_psionics(ch) )
    {
	send_to_char("You cannot use psionics.\n\r", ch);
	return;
    }

    banner_to_char("PSIONIC UPKEEPS", ch);

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
	if (ch->psi_upkeep[sn].dur != -1)
	{
	    sprintf(buf, "        %-30s :    %2d %s upkeep\n\r",
		psi_table[sn].name,
		psi_table[sn].upkeep,
		(psi_table[sn].type==PSI_FAST 
		|| psi_table[sn].type==PSI_PROGRESSIVE_FAST) ? "fast" : "slow");
	    send_to_char(buf,ch);
	    if (psi_table[sn].type==PSI_FAST
		|| psi_table[sn].type==PSI_PROGRESSIVE_FAST)
		total += psi_table[sn].upkeep * 100;
	    else
		total += psi_table[sn].upkeep * (PULSE_TICK / PULSE_VIOLENCE) * 100;
	    found = TRUE;
	}
    }

    if (!found)
    {
	send_to_char("\n\r",ch);
	banner_to_char("no psionic upkeeps",ch);
	return;
    }

    sprintf(buf,"Total: %3d / round  :  Current Psp's: %4d  :  Max Duration: %d",
	total/100, psp,  (psp)/(total/100));
    divide5_to_char(ch);
    cent_to_char(buf,ch);
    divide4_to_char(ch);
}

/* Maintain Upkeeps */

void psi_fastticker(CHAR_DATA *ch)
{
    int sn;

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
	if (ch->psi_learnt[sn]!=0 && ch->psi_upkeep[sn].dur >= 0
	    && (psi_table[sn].type == PSI_FAST
	    ||  psi_table[sn].type == PSI_PROGRESSIVE_FAST))
	{
	    ch->psp -= psi_table[sn].upkeep;
	    if (ch->psp <= 0)
	    {
		ch->psp = 0;
		(*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim,TARGET_CHAR, sn, -1);
		psi_dropupkeep(ch, sn);
		continue;
	    }
	    if (psi_table[sn].type == PSI_PROGRESSIVE_FAST)
	    {
		(*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim, TARGET_CHAR, sn,++ch->psi_upkeep[sn].dur);
	    }
	}
    }
}

void psi_slowticker(CHAR_DATA *ch)
{
    int sn;

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
	if (ch->psi_learnt[sn]!=0 && ch->psi_upkeep[sn].dur >= 0
	    && (psi_table[sn].type == PSI_SLOW
	    ||  psi_table[sn].type == PSI_PROGRESSIVE_SLOW))
	{
	    ch->psp -= psi_table[sn].upkeep;
	    if (ch->psp <= 0)
	    {
		ch->psp = 0;
		(*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim, TARGET_CHAR, sn,-1);
		psi_dropupkeep(ch, sn);
		continue;
	    }
	    if (psi_table[sn].type == PSI_PROGRESSIVE_SLOW)
		(*psi_table[sn].func) (ch, (void *) ch->psi_upkeep[sn].victim, TARGET_CHAR, sn, ++ch->psi_upkeep[sn].dur);
	}
    }
}

/* Skill List */

void do_psiskill(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int sn;
    int disc;
    int found = 0;

    if (!can_use_psionics(ch) )
    {
	send_to_char("You cannot use psionics.\n\r", ch);
	return;
    }

    if (argument[0] == 0)
    {
	int i;
	char buf[1000];

	banner2_to_char("PSIONIC DISCIPLINES", ch);
	stc("\n\r",ch);
	for ( i = 1; i < 8; i++)
	{
	    if (ch->pcdata->psi_disc[i] > 0 && psi_disc_name(i) != NULL)
	    {
		sprintf(buf, "%s%s", psi_disc_name(i),
			(psi_prime_disc(ch) == i) ? " (primary)" : "");
		buf[0] = UPPER(buf[0]);
		cent_to_char(buf, ch);
	    }
	}
	stc("\n\r",ch);
	banner2_to_char("psiskill <disipline>", ch);
	return;
    }	

    if (!str_prefix(argument, "psychometabolism"))
    {
	disc = PSI_PSYCHOMETABOLISM;
	banner2_to_char("PSYCHOMETABOLISM", ch);
    }
    else if (!str_prefix(argument, "psychokinesis"))
    {
	disc = PSI_PSYCHOKINESIS;
	banner2_to_char("PSYCHOKINESIS", ch);
    }
    else if (!str_prefix(argument, "telepathy"))
    {
	disc = PSI_TELEPATHY;
	banner2_to_char("TELEPATHY", ch);
    }
    else if (!str_prefix(argument, "psychoportation"))
    {
	disc = PSI_PSYCHOPORTATION;
	banner2_to_char("PSYCHOPORTATION", ch);
    }
    else if (!str_prefix(argument, "clairsentience"))
    {
	disc = PSI_CLAIRSENTIENCE;
	banner2_to_char("CLAIRSENTIENCE", ch);
    }
    else if (!str_prefix(argument, "metapsionics"))
    {
	disc = PSI_METAPSIONICS;
	banner2_to_char("METAPSIONICS", ch);
    }
    else
    {
	do_psiskill( ch, "" );
	return;
    }

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
	if (psi_table[sn].name != NULL && psi_table[sn].discipline == disc && ch->psi_learnt[sn] != 0)
	{
	    found += 1;
	    if (psi_table[sn].type == PSI_ONCE)
	    {
	    sprintf(buf, "    %-30s :  %3d initial  :  no upkeep    \n\r",
		psi_table[sn].name,
		psi_table[sn].init);
	    }
	    else
	    {
	    sprintf(buf, "    %-30s :  %3d initial  :  %3d %s upkeep\n\r",
		psi_table[sn].name,
		psi_table[sn].init,
		psi_table[sn].upkeep,
		(psi_table[sn].type==PSI_FAST 
		|| psi_table[sn].type==PSI_PROGRESSIVE_FAST) ? "fast" : "slow");
	    }
	    send_to_char(buf,ch);
	    found = TRUE;
	}
    }
    sprintf(buf, "%d Found", found);
    if (found == 0) sprintf(buf, "None Found");
    banner2_to_char(buf, ch);
    return;
}


/***  Nasty global variable for picky functions */

extern char *target_name; // by prool

void do_psi(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int psp;
    int sn;
    int target;

   if (!can_use_psionics(ch) )
   {
	send_to_char("You cannot use psionics.\n\r", ch);
	return;
   }

   target_name = one_argument( argument, arg1 );
   one_argument( target_name, arg2 );

   if ( arg1[0]=='\0' )
   {
	send_to_char("Use which psionic power?\n\r",ch);
	return;
   }

   if (IS_NEWFLAG(ch,AFF_FEAR))
   {
	send_to_char("You are too scared to concentrate.\n\r",ch);
	return;
   }

   if ((sn = find_psipower(ch, arg1)) < 0
   || psi_table[sn].func == NULL
   || (!IS_NPC(ch) && ch->psi_learnt[sn] == 0))
   {
	send_to_char("You don't know any powers of that name.\n\e",ch);
	return;
   }

   if ( ch->position < psi_table[sn].minimum_position )
   {
	send_to_char("You can't concentrate enough.\n\r",ch);
	return;
   }

    if (ch->psi_upkeep[sn].dur != -1)
    {
	send_to_char("You are already using this ability.\n\r",ch);
	return;
    }


   victim = NULL;
   obj = NULL;
   vo = NULL;
   target = TARGET_NONE;

   switch ( psi_table[sn].target )
   {
   default : 
	bug( "Do_psi:  bad target for sn %d.",sn);
	return;

   case TAR_IGNORE :
	break;

   case TAR_CHAR_OFFENSIVE :
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char("Use this power on whom?\n\r",ch);
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char("They aren't here.\n\r",ch);
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
	    send_to_char("You can't do that on your own follower.\n\r",ch);
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if (arg2[0]=='\0')
	{
	    victim = ch;
	}
	else
	{
	    if (( victim = get_char_room(ch, target_name ) ) == NULL)
	    {
		send_to_char("They aren't here.\n\r",ch);
		return;
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF :
	if ( arg2[0] != '\0' )
	{
	    send_to_char("You cannot cast this spell on another.\n\r",ch);
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char("What should this power be used upon?\n\r",ch);
	    return;
	}

	if ( (obj = get_obj_carry(ch, target_name, ch)) == NULL )
	{
	    send_to_char("You are not carrying that.\n\r",ch);
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
		send_to_char("Use this power on whom or what?\n\r",ch);
		return;
	    }

	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch, target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR)
	{
	    if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }
	    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	    {
		send_to_char("You can't do that on your own follower.\n\r",ch);
		return;
	    }

	    vo = (void *) victim;
	}
	else if ((obj = get_obj_here(ch, target_name)) != NULL)
	{
	    vo = (void  *) obj;
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

   case TAR_CHAR_WORLD :
	if ( arg2[0] == '\0' )
	{
	    send_to_char("Use this power on whom?\n\r",ch);
	    return;
	}
	else
	{
	    if ( ( victim = get_char_world( ch, target_name ) ) == NULL )
	    {
		send_to_char("You can't sense anybody of this name.\n\r",ch);
		return;
	    }
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;
    }


    psp = psi_table[sn].init;

    if (ch->psp < psp)
    {
	send_to_char("You don't have enough mental strength.\n\r",ch);
	return;
    }

    if (ch->level < 120)
	WAIT_STATE(ch, psi_table[sn].beats * PULSE_PER_SECOND);

    ch->psp -= psp;
    (*psi_table[sn].func) (ch, vo, TARGET_CHAR, sn, 0);

    if (psi_table[sn].type != PSI_ONCE && 
       (psi_table[sn].target == TAR_IGNORE
	|| psi_table[sn].target == TAR_CHAR_OFFENSIVE
	|| psi_table[sn].target == TAR_CHAR_DEFENSIVE
	|| psi_table[sn].target == TAR_CHAR_SELF))
    {
	if (psi_table[sn].target == TAR_IGNORE)
	    psi_addupkeep(ch, NULL, sn);
	else
	    psi_addupkeep(ch, victim, sn);
    }

    if ((psi_table[sn].target == TAR_CHAR_OFFENSIVE
    || (psi_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    && victim != ch
    && victim->master != ch
    && IS_NPC(victim))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for (vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if (victim == vch && victim == NULL )
	    {
		set_fighting(victim, ch);
		break;
	    }
	}
    }

    return;
}


/* Is using the psionic ability specified by name */

bool has_psi(CHAR_DATA *ch, char *argument)
{
    int sn;

    crash("has a psi? or just a pie");

    sn = find_psipower(ch, argument);

    if (sn < 0 || psi_table[sn].func == NULL) return FALSE;

    crash("yep just the pie");

    if (ch->psi_upkeep[sn].dur < 0)
        return FALSE;

    return TRUE;
}

/* Psionic Powers List */
/* Ability name, function name, target, init cost
	upkeep type, upkeep cost, how long delay for using abil
	discipline, level of abil, min pos */
const struct psi_type psi_table[NUM_PSI_SLOTS] =
{
    {   "synaptic overload",     psi_synaptic_overload,TAR_CHAR_OFFENSIVE,
	500,      PSI_ONCE,       0,      20,      PSI_TELEPATHY,
	10,      POS_STANDING, "", FALSE
    },

    {   "aging",     psi_aging,TAR_CHAR_OFFENSIVE,
	60,      PSI_ONCE,       0,      12,      PSI_PSYCHOMETABOLISM,
	8,      POS_STANDING, "", FALSE
    },

    {   "babble",            psi_babble,          TAR_CHAR_OFFENSIVE,
	20,     PSI_PROGRESSIVE_SLOW,   10,      2,      PSI_TELEPATHY,
	2,      POS_STANDING, "", FALSE
    },

    {   "sunrise",            psi_sunrise,TAR_CHAR_SELF,
	200,     PSI_PROGRESSIVE_FAST,   100,      2,PSI_PSYCHOMETABOLISM,
	9,      POS_SITTING, "", FALSE
    },

    {   "carapace",             psi_carapace,           TAR_CHAR_DEFENSIVE,
	5,      PSI_FAST,       2,      8,      PSI_PSYCHOMETABOLISM,
	6,      POS_STANDING, "", FALSE
    },

    {   "recall",             psi_recall,           TAR_CHAR_SELF,
	30,      PSI_FAST,       12,      8,      PSI_METAPSIONICS,
	6,      POS_SLEEPING, "", FALSE
    },

    {   "object reading",psi_object_reading,TAR_CHAR_DEFENSIVE,
	15,      PSI_SLOW,       8,      7,      PSI_CLAIRSENTIENCE,
	3,      POS_RESTING, "", FALSE
    },

    {   "regeneration" , psi_regeneration,TAR_CHAR_DEFENSIVE,
	8,      PSI_PROGRESSIVE_FAST,       4,      0,PSI_PSYCHOMETABOLISM,
	3,      POS_RESTING, "", FALSE
    },

    {   "transference",      psi_transference, TAR_CHAR_WORLD,
	300,      PSI_ONCE,       8,   8, PSI_PSYCHOPORTATION,
	9,      POS_RESTING, "", FALSE
    },

    {   "alarm",      psi_alarm,   TAR_IGNORE,
	20,      PSI_ONCE,       8,   8, PSI_CLAIRSENTIENCE,
	3,      POS_RESTING, "", FALSE
    },

    {   "mogrify",      psi_mogrify,   TAR_OBJ_INV,
	60,      PSI_ONCE,       8,   12, PSI_PSYCHOMETABOLISM,
	6,      POS_STANDING, "", FALSE
    },

    {   "success",      psi_success,   TAR_IGNORE,
	50,      PSI_ONCE,       6,   12, PSI_TELEPATHY,
	7,      POS_STANDING, "", FALSE
    },

    {   NULL,                   0,                   0,
	0,   0,                   0,   0,   0,
	0, 0, NULL, FALSE
    }
};

/* Psionic Powers List */
/* Ability name, function name, target, init cost
	upkeep type, upkeep cost, how long delay for using abil
	discipline, level of abil, min pos */

/* Psionic Ability Functions */

void psi_mogrify(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *obj2 = NULL;
    int die;
    char buf[MAX_STRING_LENGTH];

    act("You stare at $p intently.", ch, obj, NULL, TO_CHAR);
    act("$n stares at $p intently.", ch, obj, NULL, TO_ROOM);

    if (!is_name("basilisk", obj->short_descr)
	|| obj->item_type != ITEM_CORPSE_NPC)
    {
	act("$p shudders and vibrates.", ch, obj, NULL, TO_CHAR);
	act("$p shudders and vibrates.", ch, obj, NULL, TO_ROOM);
	return;
    }

    for ( ; ; )
    {
	die = dice(1, MAX_SKILL);
	if (skill_table[die].material_comp != NULL
	    && skill_table[die].material_comp[0] != '\0')
	{
	    obj2 = create_object(get_obj_index(98), 0);
	    obj2->item_type = ITEM_TRASH;
	    WRITE_STR(obj2->short_descr, 
		skill_table[die].material_comp);
	    WRITE_STR(obj2->name, 
		skill_table[die].material_comp);
	    sprintf(buf, "You see %s lying on the floor.",
		skill_table[die].material_comp);
	    WRITE_STR(obj2->description, buf);
	    break;
	}
    }
    sprintf(buf, "$p starts to pulsate, and transforms into %s.",
	obj2->short_descr);

    act(buf, ch, obj, NULL, TO_CHAR);
    act(buf, ch, obj, NULL, TO_ROOM);
    obj_to_char(obj2, ch);
    extract_obj(obj);
}

void psi_alarm(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    stc("You leave your psychic impression in the room.\n\r", ch);
    WRITE_STR(ch->in_room->alarm, GET_PROPER_NAME(ch) );
}


void psi_transference(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *room;

    if (ch->in_room == victim->in_room
	&& ch->altitude == victim->altitude)
    {
	stc("You can't transfer somebody in the same room as you.\n\r", ch);
	return;
    }
 
    if (dice(1, 5) == 1 && IS_EVIL(ch) )
    {
	stc("There is a sudden flash of light.\n\r", ch);
	char_from_room(ch);
	char_to_room(ch, get_rand_room_by_sect(SECT_HILLS), 0 );
 	act("$n appears in a cloud of orange smoke.", ch, NULL, NULL, TO_ROOM);
	return;
    }
   
    room = ch->in_room;
    stc("There is a sudden flash of light.\n\r", ch);
    stc("There is a sudden flash of light.\n\r", victim);

    char_from_room(ch);
    char_to_room(ch, victim->in_room, victim->altitude);
    
    char_from_room(victim);
    char_to_room(victim, room, ch->altitude);
 
    act("$N suddenly transforms into $n.", ch, NULL, victim, TO_NOTVICT);
    act("$N suddenly transforms into $n.", victim, NULL, ch, TO_NOTVICT);

    do_look(ch, "auto");
    do_look(victim, "auto");
}


void psi_sunrise(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    if (round == -1) return;

    if (ch->position != POS_SITTING)
    {
	send_to_char("You must be sitting down to use this power.\n\r",ch);
	return;
    }

    if (!IS_OUTSIDE(ch) )
    {
	send_to_char("You cannot do this inside.\n\r", ch);
	return;
    }

    if (time_info.hour % 24 != 6)
    {
	send_to_char("You feel dizzy and slump to the floor, totally drained.\n\r", ch);
	psi_drop_power(ch, sn);
	ch->hit *=2 / 3;
	make_unconcious(ch, 2);
	ch->move /=2;
	return;
    }

    if (round == 0)
    {
	act("The morning rays of sun cover $n with their warmth.", ch,NULL, NULL, TO_ROOM);
	send_to_char("You are healed by the sun's rays.\n\r", ch);
	ch->hit = UMIN (ch->max_hit, ch->hit + dice(5, 50) );
	ch->move = UMIN (ch->max_move, ch->move + dice(5, 50) );
    }
    if (round == 1)
    {
	act("The morning rays of sun flood over $n.",ch,NULL, NULL,TO_ROOM);
	send_to_char("You are healed by the sun's rays.\n\r", ch);
	ch->hit = UMIN (ch->max_hit, ch->hit + dice(6, 50) );
	ch->move = UMIN (ch->max_move, ch->move + dice(6, 50) );
    }
    if (round > 1)
    {
	act("$n starts to shine with all the colors of the rainbow.",ch,NULL, NULL,TO_ROOM);
	send_to_char("You feel refreshed.\n\r", ch);
	ch->hit = UMIN (ch->max_hit, ch->hit + dice(8, 50) );
	ch->move = UMIN (ch->max_move, ch->move + dice(8, 50) );
    }



}

void psi_babble(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (round == 0)
    {
	if (victim->psi_level > ch->psi_level)
	{
	    send_to_char("Their mind is too strong to affect.\n\r", ch);
	    return;
	}
	act("You stare intently at $N, confusing $S mind.",ch,NULL,victim,TO_CHAR);
	SET_BIT(victim->flag3, AFF_BABBLE);
	return;
    }
    if (round == -1)
    {
	act("You break your telepathic link with $N.",ch,NULL,victim,TO_CHAR);
	REMOVE_BIT(victim->flag3, AFF_BABBLE);
	return;
    }
}  

void psi_read_aura(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *col, *comp;
    int con;
    if (round == 0)
    {
	act("You peer intently at $N, your powers making their aura visible.",ch,NULL,victim,TO_CHAR);
	act("$n's eyes sparkle slightly.",ch,NULL,NULL,TO_ROOM);
	return;
    }
    if (round == -1)
    {
	act("The glowing aura around $N fades.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (round == 3)
    {
	psi_drop_power(ch, sn);
	return;
    }

    switch(round)
    {
    case 1 : 
	con = (((victim->hit)*100) / (UMAX(1, ch->hit)));
	col = str_dup("red");
	break;
    case 2 : 
	con = (((victim->move)*100) / (UMAX(1, ch->move)));
	col = str_dup("green");
	break;
    default : return;
    }

    if (con > 200)
	comp = str_dup("incredibly brighter than");
    else if (con > 180)
	comp = str_dup("greatly brighter than");    
    else if (con > 160)
	comp = str_dup("much brighter than");    
    else if (con > 140)
	comp = str_dup("fairly brighter than");    
    else if (con > 120)
	comp = str_dup("brighter than");    
    else if (con > 100)
	comp = str_dup("slightly brighter than");
    else if (con == 100)
	comp = str_dup("of equal brightness to");
    else if (con < 20)
	comp = str_dup("incredibly dimmer than");
    else if (con < 40)
	comp = str_dup("much dimmer than");
    else if (con < 60)
	comp = str_dup("much dimmer than");
    else if (con < 80)
	comp = str_dup("dimmer than");
    else if (con < 100)
	comp = str_dup("slightly dimmer than");
    else
	comp = str_dup("My name is chris and i have FAILED yet again!");

    sprintf(buf,"$N's %s aura is %s yours.",col,comp);
    act(buf, ch, NULL, victim, TO_CHAR);
}

void psi_synaptic_overload(CHAR_DATA *ch, void *vo, int target, int sn,int round)
{
    int dam;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    dam = number_range( 2000, 2500);
    act("$n closes his eyes and concentrates intensely.", ch, NULL, NULL,TO_ROOM);
    send_to_char("You close your eyes and concentrate.\n\r", ch);
    if (dice(1,10) == 1)
    {
	send_to_char("Your brain starts to boil....\n\r", ch);
	victim = ch;
    }
    act("$n screams in agony and drops to the floor.", victim,NULL,NULL,TO_ROOM);
    send_to_char("You scream in agony and drop to the floor, your mind crushed.\n\r", victim);
    victim->position = POS_RESTING;
    damage_old(ch, victim, dam, sn, DAM_MENTAL, FALSE);
    return;
}

void psi_aging(CHAR_DATA *ch, void *vo, int target, int sn,int round)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_NPC(ch)) return;

    act("$n closes his eyes and concentrates intensely.", ch, NULL, NULL,TO_ROOM);
    stc("You close your eyes and concentrate.\n\r", ch);

    if (!can_age(victim) || IS_NPC(victim))
    {
	stc("They appear unaffected.\n\r", ch);
	return;
    }

    if (dice(1,10) == 1)
    {
	send_to_char("You begin to age and wither.\n\r", ch);
	ch->pcdata->extra_age += dice(1, 12);
	return;
    }

    stc("You feel tired all of a sudden.\n\r", victim);
    act("$n starts to look older.", victim, NULL, NULL, TO_ROOM);
    victim->pcdata->extra_age += dice(1, 4);

    return;
}

void psi_success(CHAR_DATA *ch, void *vo, int target, int sn,int round)
{
    if (IS_FLAG5(ch, AFF5_SUCCESS))
    {
	stc("Your success is already guaranteed.\n\r", ch);
	return;
    }

    send_to_char("You feel self-assured.\n\r", ch);
    SET_BIT(ch->flag5, AFF5_SUCCESS);
}

void psi_object_reading(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    if (round == -1)
    {
	send_to_char("You are less sensitive to objects.\n\r",ch);
	REMOVE_BIT(ch->flag3, AFF_OBJECT_READING);
        return;
    }

    if (round == 0)
    {
	send_to_char("You are now more sensitive to objects.\n\r",ch);
	SET_BIT(ch->flag3, AFF_OBJECT_READING);
    }
}

void psi_carapace(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    if (round == -1)
    {
	send_to_char("Your skin softens.\n\r",ch);
	act("$n's skin regains its normal texture.",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_SET(ch->imm_flags, IMM_BASH))
    {
	send_to_char("You are already immune to bashing weapons.\n\r", ch);
	psi_dropupkeep(ch, sn);
	return;
    }   

    if (round == 0)
    {
	send_to_char("Your skin hardens into a thick shell.\n\r",ch);
	act("$n's skin hardens into a thick shell.",ch,NULL,NULL,TO_ROOM);
    }

}

void psi_recall(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    if (round == -1)
    {
	send_to_char("You stop using the recall power.\n\r",ch);
	REMOVE_BIT(ch->flag4, AFF_PSI_RECALL);
	return;
    }

    if (IS_FLAG4(ch, AFF_PSI_RECALL))
    {
	send_to_char("You are already using the recall power.\n\r", ch);
	psi_dropupkeep(ch, sn);
	return;
    }   

    if (round == 0)
    {
	send_to_char("You feel mentally acute.\n\r",ch);
	SET_BIT(ch->flag4, AFF_PSI_RECALL);
    }

}

void psi_regeneration(CHAR_DATA *ch, void *vo, int target, int sn, int round)
{
    if (round == -1)
    {
	send_to_char("Your skin stops tingling.\n\r",ch);
	return;
    }

    if (round == 0)
	send_to_char("Your skin starts to tingle slightly.\n\r",ch);

    ch->hit += 4;

    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
}

