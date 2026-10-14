/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hell.h"
#include "magic.h"
#include "recycle.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups	);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

void disp_skills 	args( (CHAR_DATA *ch, int type) );

void do_innate( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int sn;
    buf[0] = 0;

    if (!GOT_INNATE(ch))
    {
	stc("You cannot use innate abilities.\n\r",ch);
    }

    banner2_to_char("INNATE ABILITIES", ch);

    for (sn = 0; sn < MAX_SKILL; sn++)
	if (ch->pcdata->innate_max[sn] > 0) found = TRUE;

    if (!found)
    {
	cent_to_char("You have no innate abilities.",ch);
	divide6_to_char(ch);
	return;
    }

    sprintf(buf, "Spell / Ability                  Max (Current) Uses       Regain Rate  ");
    cent_to_char(buf, ch);
    stc("\n\r",ch);

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (ch->pcdata->innate_max[sn] <= 0)
		continue;
	sprintf(buf, "%-30s         %d (%d)          1 per %3d hour%c",
	    skill_table[sn].name,
	    ch->pcdata->innate_max[sn],
	    ch->pcdata->innate[sn],
	    ch->pcdata->innate_regain[sn],
	    (ch->pcdata->innate_regain[sn] > 1) ? 's' : ' ');
	cent_to_char(buf, ch);
    }

    divide6_to_char(ch);
}

void do_skills (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    if (IS_NPC(ch))
    {
	stc("You cannot use the skills command in this form.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	cent_to_char("---=-==>> Skill Groups <<==-=----", ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Your skill in weapons is currently at level %d.",
		ch->pcdata->treelearn[SKILL_WEAPON]);
	cent_to_char(buf, ch);
	sprintf(buf, "Your combat skill is currently at level %d.",
		ch->pcdata->treelearn[SKILL_COMBAT]);
	cent_to_char(buf, ch);
	sprintf(buf, "Your skill as a thief is currently %d."
		,ch->pcdata->treelearn[SKILL_THIEF]);
	cent_to_char(buf, ch);
	sprintf(buf, "Your skill as a priest is currently %d.",
		ch->pcdata->treelearn[SKILL_PRIEST]);
	cent_to_char(buf, ch);
	sprintf(buf, "Your skill as a wizard is currently %d.",
		ch->pcdata->treelearn[SKILL_WIZARD]);
	cent_to_char(buf, ch);
	sprintf(buf, "Your skill as a knight is currently %d.",
		ch->pcdata->treelearn[SKILL_KNIGHT]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);
	divide6_to_char(ch);
	return;
    }

    if (!str_prefix(arg1, "combat"))
    {
        cent_to_char("---==-===> Skills <===-==---", ch);
        stc("\n\r", ch);
        disp_skills(ch, SKILL_COMBAT);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }
    else if (!str_prefix(arg1, "weapons"))
    {
        cent_to_char("---==-===> Skills <===-==---", ch);
        stc("\n\r", ch);
	disp_skills(ch, SKILL_WEAPON);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }
    else if (!str_prefix(arg1, "thief"))
    {
        cent_to_char("---==-===> Skills <===-==---", ch);
        stc("\n\r", ch);
	disp_skills(ch, SKILL_THIEF);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }
    else if (!str_prefix(arg1, "knight"))
    {
	cent_to_char("---==-===> Skills <===-==---", ch);
   	stc("\n\r", ch);
	disp_skills(ch, SKILL_KNIGHT);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }
    else if (!str_prefix(arg1, "priest"))
    {
        cent_to_char("---==-===> Skills <===-==---", ch);
        stc("\n\r", ch);
	disp_skills(ch, SKILL_PRIEST);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }
    else if (!str_prefix(arg1, "wizard"))
    {
        cent_to_char("---==-===> Skills <===-==---", ch);
        stc("\n\r", ch);
	disp_skills(ch, SKILL_WIZARD);
	stc("\n\r", ch);
	divide6_to_char(ch);
	return;
    }
    else do_skills(ch, "");
}

void disp_skills(CHAR_DATA *ch, int type)
{
    char buf[MAX_STRING_LENGTH];

    if (type == SKILL_WEAPON)
    {
	sprintf(buf, "Weaponmastery skill : %d.", 
	    ch->pcdata->treelearn[SKILL_WEAPON]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Slashing weapons : %d.",
	    ch->pcdata->treelearn[SKILL_WEAPON]
	    + ch->pcdata->treelearn[SKILL_WEAP_SLASH]);
	cent_to_char(buf, ch);
	sprintf(buf, "Axe : %s    Sword : %s    Polearm : %s",
	    score_word[get_skill(ch, gsn_axe)/5],
	    score_word[get_skill(ch, gsn_sword)/5],
	    score_word[get_skill(ch, gsn_polearm)/5] );
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Bashing weapons : %d.",
	    ch->pcdata->treelearn[SKILL_WEAPON]
	    + ch->pcdata->treelearn[SKILL_WEAP_BASH]);
	cent_to_char(buf, ch);
	sprintf(buf, "Whip : %s  Mace : %s  Flail : %s  Brawling : %s",
	    score_word[get_skill(ch, gsn_whip)/5],
	    score_word[get_skill(ch, gsn_mace)/5],
	    score_word[get_skill(ch, gsn_flail)/5],
	    score_word[get_skill(ch, gsn_hand_to_hand)/5] );
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Piercing weapons : %d.",
	    ch->pcdata->treelearn[SKILL_WEAPON]
	    + ch->pcdata->treelearn[SKILL_WEAP_STAB]);
	cent_to_char(buf, ch);
	sprintf(buf, "Spear : %s    Dagger : %s",
	    score_word[get_skill(ch, gsn_spear)/5],
	    score_word[get_skill(ch, gsn_dagger)/5]);
	cent_to_char(buf, ch);
	return;
    }
    else if (type == SKILL_THIEF)
    {
	sprintf(buf, "Thievery skill : %d.", 
	    ch->pcdata->treelearn[SKILL_THIEF]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Assassin : %d.",
	    ch->pcdata->treelearn[SKILL_THIEF]
	    + ch->pcdata->treelearn[SKILL_THIE_ASSASSIN]);
	cent_to_char(buf, ch);
	sprintf(buf, "Envenom : %s    Backstab : %s",
	    score_word[get_skill(ch, gsn_envenom)/5],
	    score_word[get_skill(ch, gsn_backstab)/5]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Rogue : %d.",
	    ch->pcdata->treelearn[SKILL_THIEF]
	    + ch->pcdata->treelearn[SKILL_THIE_ROGUE]);
	cent_to_char(buf, ch);
	sprintf(buf, "Haggle : %s    Hide : %s    Peek : %s",
	    score_word[get_skill(ch, gsn_haggle)/5],
	    score_word[get_skill(ch, gsn_hide)/5],
	    score_word[get_skill(ch, gsn_peek)/5] );
	cent_to_char(buf, ch);
	sprintf(buf, "Pick Locks : %s    Sneak : %s    Steal : %s",
	    score_word[get_skill(ch, gsn_pick_lock)/5],
	    score_word[get_skill(ch, gsn_sneak)/5],
	    score_word[get_skill(ch, gsn_steal)/5] );
	cent_to_char(buf, ch);
	sprintf(buf, "Tumbling : %s    Appraisal : %s",
	    score_word[get_skill(ch, gsn_tumbling)/5],
	    score_word[get_skill(ch, gsn_appraisal)/5] );
	cent_to_char(buf, ch);

	return;
    }
    else if (type == SKILL_PRIEST)
    {
	sprintf(buf, "Priestly skill : %d.", 
	    ch->pcdata->treelearn[SKILL_PRIEST]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Healer : %d.",
	    ch->pcdata->treelearn[SKILL_PRIEST]
	    + ch->pcdata->treelearn[SKILL_PRIE_HEALING]);
	cent_to_char(buf, ch);
	sprintf(buf, "Fast Healing : %s    Meditation : %s",
	    score_word[get_skill(ch, gsn_fast_healing)/5],
	    score_word[get_skill(ch, gsn_meditation)/5]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Craftsman : %d.",
	    ch->pcdata->treelearn[SKILL_PRIEST]
	    + ch->pcdata->treelearn[SKILL_PRIE_CRAFT]);
	cent_to_char(buf, ch);
	sprintf(buf, "Foraging : %s",
	    score_word[get_skill(ch, gsn_foraging)/5]);
	cent_to_char(buf, ch);

	return;
    }
    else if (type == SKILL_WIZARD)
    {
	sprintf(buf, "Wizardly skill : %d.", 
	    ch->pcdata->treelearn[SKILL_WIZARD]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Artificier : %d.",
	    ch->pcdata->treelearn[SKILL_WIZARD]
	    + ch->pcdata->treelearn[SKILL_WIZA_ITEMS]);
	cent_to_char(buf, ch);
	sprintf(buf, "Scrolls : %s    Staves : %s    Wands : %s",
	    score_word[get_skill(ch, gsn_scrolls)/5],
	    score_word[get_skill(ch, gsn_staves)/5],
	    score_word[get_skill(ch, gsn_wands)/5]);
	cent_to_char(buf, ch);
	sprintf(buf, "Lore : %s",
	    score_word[get_skill(ch, gsn_lore)/5]);
	cent_to_char(buf, ch);

	return;
    }
    else if (type == SKILL_KNIGHT)
    {
	sprintf(buf, "Knightly skill : %d.", 
	    ch->pcdata->treelearn[SKILL_KNIGHT]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Warrior : %d.",
	    ch->pcdata->treelearn[SKILL_KNIGHT]
	    + ch->pcdata->treelearn[SKILL_KNIG_COMBAT]);
	cent_to_char(buf, ch);
	sprintf(buf, "Blind Fighting : %s    Berserking : %s",
	    score_word[get_skill(ch, gsn_blindfighting)/5],
	    score_word[get_skill(ch, gsn_berserk)/5]);
	cent_to_char(buf, ch);
	sprintf(buf, "Battle Prowess : %s",
	    score_word[get_skill(ch, gsn_enhanced_damage)/5]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Path of the Smith : %d.",
	    ch->pcdata->treelearn[SKILL_KNIGHT]
	    + ch->pcdata->treelearn[SKILL_KNIG_FORGING]);
	cent_to_char(buf, ch);
	sprintf(buf, "Blacksmithing : %s    Repair : %s",
	    score_word[get_skill(ch, gsn_blacksmith)/5],
	    score_word[get_skill(ch, gsn_repair)/5]);
	cent_to_char(buf, ch);

	return;
    }
    else if (type == SKILL_COMBAT)
    {
	sprintf(buf, "Combat skill : %d.", 
	    ch->pcdata->treelearn[SKILL_COMBAT]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Attacking : %d.",
	    ch->pcdata->treelearn[SKILL_COMBAT]
	    + ch->pcdata->treelearn[SKILL_COMB_ATTACK]);
	cent_to_char(buf, ch);
	sprintf(buf, "Bashing : %s    Dirt Kicking : %s    Kicking : %s",
	    score_word[get_skill(ch, gsn_bash)/5],
	    score_word[get_skill(ch, gsn_dirt)/5],
	    score_word[get_skill(ch, gsn_kick)/5] );
	cent_to_char(buf, ch);
	sprintf(buf, "Tripping : %s",
	    score_word[get_skill(ch, gsn_trip)/5]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Defending : %d.",
	    ch->pcdata->treelearn[SKILL_COMBAT]
	    + ch->pcdata->treelearn[SKILL_COMB_DEFENSE]);
	cent_to_char(buf, ch);
	sprintf(buf, "Shield Block : %s    Parrying : %s",
	    score_word[get_skill(ch, gsn_shield_block)/5],
	    score_word[get_skill(ch, gsn_parry)/5] );
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);

	sprintf(buf, "Agility : %d.",
	    ch->pcdata->treelearn[SKILL_COMBAT]
	    + ch->pcdata->treelearn[SKILL_COMB_AGILITY]);
	cent_to_char(buf, ch);
	sprintf(buf, "Disarming : %s    Dodging : %s    Rescue : %s",
	    score_word[get_skill(ch, gsn_disarm)/5],
	    score_word[get_skill(ch, gsn_dodge)/5],
	    score_word[get_skill(ch, gsn_rescue)/5] );
	cent_to_char(buf, ch);
	sprintf(buf, "Off Hand : %s   Archery : %s   Throwing : %s",
	    score_word[get_skill(ch, gsn_off_hand)/5], 
	    score_word[get_skill(ch, gsn_archery)/5], 
	    score_word[get_skill(ch, gsn_throwing)/5] );
	cent_to_char(buf, ch);
	return;
    }
    bug("Disp skills - weird number", 0);
    return;
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    char buf[100];
    int chance = 0, newbie;

    crash("check improve function");

    if (IS_NPC(ch))
	return;

    SS(0);

    /* Stupid crashing thing */
    if (get_treelev(sn, 1) == SKILL_FUCKED) return;

    /* Trial and error... that is the way to learn */
    if (success) return; 

    if (!str_cmp(skill_table[sn].name, "reserved"))
	return;

    /* Adam is doing it a different way... okay! */

    if (ch->pcdata->learned[sn] > 50) return;
    /* This means cant go above 50... but maybe they can in future? */

    chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
    chance += ch->generation;

    SS(1);

    newbie = 200 - ch->max_hit;
    if (newbie < 0) newbie = 0;
    chance += 3 * newbie;

// newbie bonus by Chris.  Ain't I nice :) No you fuckhead.

    if (dice(1,3000) > chance && skill_table[sn].spell_fun == spell_null)
	return;

    if (dice(1,500) > chance && skill_table[sn].spell_fun !=spell_null)
	return;

    SS(2);

    if (number_percent()*2/3 > ch->pcdata->learned[sn])
    {
	sprintf(buf, "You have got better at %s.\n\r",skill_table[sn].name);
        send_to_char(buf,ch);

	if (skill_table[sn].spell_fun != spell_null)
	    ch->pcdata->learned[sn] += dice(2, 4);
        else ch->pcdata->learned[sn]++;
	/* Spells are quicker to learn than other things */
    SS(3);

        ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],50);
        gain_exp(ch, 100);

	/* Ok.. the fun bit.  This is gaining actual skill levels 
	   Firstly.. the level1, this should rarely happen */
  	if (TIME_UP(ch, TIMER_SKILL_LEV1))
	{
	    if (dice(1, 8) > 1) return; /* breaks out */
	    SET_TIMER(ch, TIMER_SKILL_LEV1,
		ch->pcdata->treelearn[get_treelev(sn, 1)] * 
		ch->pcdata->treelearn[get_treelev(sn, 1)] + 200);
	    ch->pcdata->treelearn[get_treelev(sn, 1)]++;
	    if (ch->pcdata->treelearn[get_treelev(sn, 1)] > 20)
		ch->pcdata->treelearn[get_treelev(sn, 1)] = 20;
	    gain_exp(ch, 5000);
    SS(4);
	    stc("You feel much more experienced.\n\r", ch);

	    /* This is the warlock autopromote */
	    if (get_treelev(sn, 1) == SKILL_WIZARD
		&& ch->secondaryrace == RACE_WARLOCK
		&& ch->generation < 12
		&& ch->pcdata->treelearn[SKILL_WIZARD] > 12)
	    {
		ch->generation++;
		stc("Congratulations.  You have gained a rank.\n\r", ch);
	    }
    SS(5);
	
	    return;
	}

  	if (TIME_UP(ch, TIMER_SKILL_LEV2))
	{
	    if (dice(1, 4) > 1) return; /* breaks out */
	    SET_TIMER(ch, TIMER_SKILL_LEV2,
		ch->pcdata->treelearn[get_treelev(sn, 2)] *
		ch->pcdata->treelearn[get_treelev(sn, 2)] + 20);
	    ch->pcdata->treelearn[get_treelev(sn, 2)]++;
	    if (ch->pcdata->treelearn[get_treelev(sn, 2)] > 30)
		ch->pcdata->treelearn[get_treelev(sn, 2)] = 30;
	    gain_exp(ch, 1000);
    SS(6);
	    stc("You feel more experienced.\n\r", ch);
	    return;
	}
    }
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}

/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn,gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 1;
	}
	return;
    }

    /* now check groups */

    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)
	{
	    ch->pcdata->group_known[gn] = TRUE;
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;

     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	return;
    }
 
    /* now check groups */
 
    gn = group_lookup(name);
 
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}



int group_lookup( const char *name )
{
    int gn;

    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }
 
    return -1;
}


/*
 * Spell command, rewritten by Jander.  OH yes, mortals.
 */


void do_spells(CHAR_DATA *ch, char *argument)
{
    int  sn,wrap, level, clevel = -1, mlevel = -1;
    char buf[MAX_STRING_LENGTH];
    bool dispall,found = FALSE, disp = FALSE, print = TRUE;
    bool foundspell = FALSE;
    char arg2[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);  
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch))
    {
	send_to_char("Hey, all your spells are automatic.\n\r", ch);
	return;
    }

    if (!can_cast_flame(ch)
	&& !can_cast_air(ch)
	&& !can_cast_entropy(ch)
	&& !can_cast_life(ch)
	&& !can_cast_earth(ch)
	&& !can_cast_water(ch))
    {
	send_to_char("You cannot cast any spells yet.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	int wrap = 0, i, j;

	cent_to_char("Memorized Spells", ch);
	cent_to_char("-=[**]=--=[**]=-", ch);
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
		if (ch->pcdata->memorize[i][j] != -1)
		{
		    sprintf(buf, "   %-17s (%3d)", skill_table[ch->pcdata->memorize[i][j]].name, get_skill(ch, ch->pcdata->memorize[i][j]));    
		    send_to_char(buf, ch);
		    foundspell = TRUE;
		    wrap += 1;
		}
	    }
	}
	if (!foundspell)
	    cent_to_char("You have no spells memorized.\n\r", ch);

	else if (wrap != 0) send_to_char("\n\r", ch);
	return;
    }

    dispall = FALSE;

    if (!is_number(arg1) && is_number(arg2) )
    {
	clevel = atoi(arg2);
	if (clevel > 10 || clevel < 1)
	{
	    send_to_char("Level range is from 1 to 10.\n\r", ch);
	    return;
	}
    }	

    if (is_number(arg1) && is_number(arg2) )
    {
	clevel = atoi(arg1);
	if (clevel > 10 || clevel < 1)
	{
	    send_to_char("Level range is from 1 to 10.\n\r", ch);
	    return;
	}
	mlevel = atoi(arg2);
	if (mlevel > 10 || mlevel < 1)
	{
	    send_to_char("Level range is from 1 to 10.\n\r", ch);
	    return;
	}
	dispall = TRUE;
    }

    else if (is_number(arg1) && !is_number(arg2) )
    {
	clevel = atoi(arg1);
	if (clevel > 10 || clevel < 1)
	{
	    send_to_char("Level range is from 1 to 10.\n\r", ch);
	    return;
	}
	dispall = TRUE;
    }

    if ((!str_prefix(arg1, "flame") || dispall==TRUE)
	&& can_cast_flame(ch))
    {
	disp = TRUE;
        if (print) send_to_char("Learnt spells:\n\n\r",ch);
	print = FALSE;
        send_to_char("#1Flame:#n\n\r",ch);
	send_to_char("#1*****#n\n\r",ch);
        wrap = 0;
	for (level = 1 ; level < 11 ; level++)
	{
	    if (clevel != -1 && clevel != level && mlevel == -1) continue;
	    else if (mlevel == clevel && clevel != level
		&& mlevel != -1 && clevel != -1) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel > clevel
		&& (level < clevel || level > mlevel) ) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel < clevel
		&& (level > clevel || level < mlevel) ) continue;

            for (sn = 0; sn < MAX_SKILL; sn++)
            {
                if (skill_table[sn].name == NULL)
                    break;
 
		if (ch->pcdata->learned[sn]>0
                    && skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].level == level
                    && skill_table[sn].realm == 1)
                {
            	    found = TRUE;
		    foundspell = TRUE;
       		    sprintf(buf,"%2d. %-20s",skill_table[sn].level,skill_table[sn].name);
                    send_to_char(buf, ch);
                    wrap += 1;
		    if (wrap == 0) foundspell = FALSE;
		    if (wrap == 3)
		    {
			send_to_char("\n\r", ch);
			wrap = 0;
		    }
                }
	    }
	    if (wrap != 0 && foundspell) send_to_char("\n\r", ch); 
	    foundspell = FALSE;
	    wrap = 0;
        }
        if (!found)
            send_to_char("    You know no spells in this realm.\n\r",ch);
            send_to_char("#n\n\r",ch);
    }

    found = FALSE;
    if ((!str_prefix(arg1, "water") || dispall==TRUE)
	&& can_cast_water(ch))
    {
	disp = TRUE;
        if (print) send_to_char("Learnt spells:\n\n\r",ch);
	print = FALSE;
        send_to_char("#4Water:#n\n\r",ch);
	send_to_char("#4~~~~~#n\n\r",ch);
        wrap = 0;
	for (level = 1 ; level < 11 ; level++)
	{
	    if (clevel != -1 && clevel != level && mlevel == -1) continue;
	    else if (mlevel == clevel && clevel != level
		&& mlevel != -1 && clevel != -1) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel > clevel
		&& (level < clevel || level > mlevel) ) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel < clevel
		&& (level > clevel || level < mlevel) ) continue;

            for (sn = 0; sn < MAX_SKILL; sn++)
            {
                if (skill_table[sn].name == NULL)
                    break;
 
		if (ch->pcdata->learned[sn]>0
                    && skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].level == level
                    && skill_table[sn].realm == 2)
                {
            	    found = TRUE;
		    foundspell = TRUE;
       		    sprintf(buf,"%2d. %-20s",skill_table[sn].level,skill_table[sn].name);
                    send_to_char(buf, ch);
                    wrap += 1;
		    if (wrap == 0) foundspell = FALSE;
		    if (wrap == 3)
		    {
			send_to_char("\n\r", ch);
			wrap = 0;
		    }
                }
	    }
	    if (wrap != 0 && foundspell) send_to_char("\n\r", ch); 
	    foundspell = FALSE;
	    wrap = 0;
        }
        if (!found)
            send_to_char("    You know no spells in this realm.\n\r",ch);
            send_to_char("#n\n\r",ch);
    }

    found = FALSE;

    if ((!str_prefix(arg1, "air") || dispall==TRUE)
	&& can_cast_air(ch))
    {
	disp = TRUE;
        if (print) send_to_char("Learnt spells:\n\n\r",ch);
	print = FALSE;
        send_to_char("#6Air:#n\n\r",ch);
	send_to_char("#6===#n\n\r",ch);
        wrap = 0;
	for (level = 1 ; level < 11 ; level++)
	{
	    if (clevel != -1 && clevel != level && mlevel == -1) continue;
	    else if (mlevel == clevel && clevel != level
		&& mlevel != -1 && clevel != -1) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel > clevel
		&& (level < clevel || level > mlevel) ) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel < clevel
		&& (level > clevel || level < mlevel) ) continue;

            for (sn = 0; sn < MAX_SKILL; sn++)
            {
                if (skill_table[sn].name == NULL)
                    break;
 
		if (ch->pcdata->learned[sn]>0
                    && skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].level == level
                    && skill_table[sn].realm == 3)
                {
            	    found = TRUE;
		    foundspell = TRUE;
       		    sprintf(buf,"%2d. %-20s",skill_table[sn].level,skill_table[sn].name);
                    send_to_char(buf, ch);
                    wrap += 1;
		    if (wrap == 0) foundspell = FALSE;
		    if (wrap == 3)
		    {
			send_to_char("\n\r", ch);
			wrap = 0;
		    }
                }
	    }
	    if (wrap != 0 && foundspell) send_to_char("\n\r", ch); 
	    foundspell = FALSE;
	    wrap = 0;
        }
        if (!found)
            send_to_char("    You know no spells in this realm.\n\r",ch);
            send_to_char("#n\n\r",ch);
    }

    found = FALSE;
    if ((!str_prefix(arg1, "earth") || dispall==TRUE)
	&& can_cast_earth(ch))
    {
	disp = TRUE;
        if (print) send_to_char("Learnt spells:\n\n\r",ch);
	print = FALSE;
        send_to_char("#2Earth:#n\n\r",ch);
	send_to_char("#2###########n\n\r",ch);
        wrap = 0;
	for (level = 1 ; level < 11 ; level++)
	{
	    if (clevel != -1 && clevel != level && mlevel == -1) continue;
	    else if (mlevel == clevel && clevel != level
		&& mlevel != -1 && clevel != -1) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel > clevel
		&& (level < clevel || level > mlevel) ) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel < clevel
		&& (level > clevel || level < mlevel) ) continue;

            for (sn = 0; sn < MAX_SKILL; sn++)
            {
                if (skill_table[sn].name == NULL)
                    break;
 
		if (ch->pcdata->learned[sn]>0
                    && skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].level == level
                    && skill_table[sn].realm == 4)
                {
            	    found = TRUE;
		    foundspell = TRUE;
       		    sprintf(buf,"%2d. %-20s",skill_table[sn].level,skill_table[sn].name);
                    send_to_char(buf, ch);
                    wrap += 1;
		    if (wrap == 0) foundspell = FALSE;
		    if (wrap == 3)
		    {
			send_to_char("\n\r", ch);
			wrap = 0;
		    }
                }
	    }
	    if (wrap != 0 && foundspell) send_to_char("\n\r", ch); 
	    foundspell = FALSE;
	    wrap = 0;
        }
        if (!found)
            send_to_char("    You know no spells in this realm.\n\r",ch);
            send_to_char("#n\n\r",ch);
    }

    found = FALSE;
    if ((!str_prefix(arg1, "entropy") || dispall==TRUE)
	&& can_cast_entropy(ch))
    {
	disp = TRUE;
        if (print) send_to_char("Learnt spells:\n\n\r",ch);
	print = FALSE;
        send_to_char("#5Entropy:#n\n\r",ch);
	send_to_char("#5+++++++#n\n\r",ch);
        wrap = 0;
	for (level = 1 ; level < 11 ; level++)
	{
	    if (clevel != -1 && clevel != level && mlevel == -1) continue;
	    else if (mlevel == clevel && clevel != level
		&& mlevel != -1 && clevel != -1) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel > clevel
		&& (level < clevel || level > mlevel) ) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel < clevel
		&& (level > clevel || level < mlevel) ) continue;

            for (sn = 0; sn < MAX_SKILL; sn++)
            {
                if (skill_table[sn].name == NULL)
                    break;
 
		if (ch->pcdata->learned[sn]>0
                    && skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].level == level
                    && skill_table[sn].realm == 5)
                {
		    foundspell = TRUE;
            	    found = TRUE;
       		    sprintf(buf,"%2d. %-20s",skill_table[sn].level,skill_table[sn].name);
                    send_to_char(buf, ch);
                    wrap += 1;
		    if (wrap == 0) foundspell = FALSE;
		    if (wrap == 3)
		    {
			send_to_char("\n\r", ch);
			wrap = 0;
		    }
                }
	    }
	    if (wrap != 0 && foundspell) send_to_char("\n\r", ch); 
	    foundspell = FALSE;
	    wrap = 0;
        }
        if (!found)
            send_to_char("    You know no spells in this realm.\n\r",ch);
            send_to_char("#n\n\r",ch);
    }

    found = FALSE;
    if ((!str_prefix(arg1, "life") || dispall==TRUE)
	&& can_cast_life(ch))
    {
	disp = TRUE;
        if (print) send_to_char("Learnt spells:\n\n\r",ch);
	print = FALSE;
        send_to_char("Life:\n\r",ch);
	send_to_char("<><>\n\r",ch);
        wrap = 0;
	for (level = 1 ; level < 11 ; level++)
	{
	    if (clevel != -1 && clevel != level && mlevel == -1) continue;
	    else if (mlevel == clevel && clevel != level
		&& mlevel != -1 && clevel != -1) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel > clevel
		&& (level < clevel || level > mlevel) ) continue;
	    else if (mlevel != -1 && clevel != -1 && mlevel < clevel
		&& (level > clevel || level < mlevel) ) continue;

            for (sn = 0; sn < MAX_SKILL; sn++)
            {
                if (skill_table[sn].name == NULL)
                    break;
 
		if (ch->pcdata->learned[sn]>0
                    && skill_table[sn].spell_fun != spell_null
		    && skill_table[sn].level == level
                    && skill_table[sn].realm == REALM_LIFE)
                {
		    foundspell = TRUE;
            	    found = TRUE;
       		    sprintf(buf,"%2d. %-20s",skill_table[sn].level,skill_table[sn].name);
                    send_to_char(buf, ch);
                    wrap += 1;
		    if (wrap == 0) foundspell = FALSE;
		    if (wrap == 3)
		    {
			send_to_char("\n\r", ch);
			wrap = 0;
		    }
                }
	    }
	    if (wrap != 0 && foundspell) send_to_char("\n\r", ch); 
	    foundspell = FALSE;
	    wrap = 0;
        }
        if (!found)
            send_to_char("    You know no spells in this realm.\n\r",ch);
            send_to_char("#n\n\r",ch);
    }

    if (!disp && can_cast_life(ch) )
    {
	send_to_char("Syntax : spells [flame water air earth entropy life]\n\r", ch); 
	send_to_char("         spells <level>\n\r", ch);
	send_to_char("         spells <lower range> <upper range>\n\r", ch);
    }

    else if (!disp )
    {
	send_to_char("Syntax : spells [flame water air earth entropy]\n\r", ch); 
	send_to_char("         spells <level>\n\r", ch);
	send_to_char("         spells <lower range> <upper range>\n\r", ch);
    }
}


