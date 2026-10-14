/*********************************************************************
 * New and Cunning Combat Routines - (C) 1997 Chris Mellish          *
 *********************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"

#define MAX_DIST 20
#define WEAPON_DAM 2

/* command procedures needed */
DECLARE_DO_FUN(interpret        );
DECLARE_DO_FUN(do_backstab      );
DECLARE_DO_FUN(do_emote         );
DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_bash          );
DECLARE_DO_FUN(do_trip          );
DECLARE_DO_FUN(do_dirt          );
DECLARE_DO_FUN(do_flee          );
DECLARE_DO_FUN(do_kick          );
DECLARE_DO_FUN(do_disarm        );
DECLARE_DO_FUN(do_get           );
DECLARE_DO_FUN(do_yell          );
DECLARE_DO_FUN(do_sacrifice     );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_wear          );
DECLARE_DO_FUN(do_crush         );
DECLARE_DO_FUN(do_fightstyle    );

void do_fsmenu( CHAR_DATA *ch, char *argument )
{
    CHECK_MENU(ch);

    menu_init_flags( ch, &ch->fight_style );
    menu_option( ch, 0, "Only block attacks, never counter attack", NULL, MENU_FLAG, FS_BLOCK, 1);
    menu_option( ch, 1, "Only attack, never block", NULL, MENU_FLAG, FS_ATTACK, 1);
    menu_option( ch, 2, "Attack only when you can block as well", NULL, MENU_FLAG, FS_CONSERVATIVE, 1);
    menu_option( ch, 3, "Attack and block whenever possible", NULL, MENU_FLAG, FS_AGGRESSIVE, 1);
    menu_divider( ch, 4 );
    menu_option( ch, 5, "Concentrate all attacks on one foe", NULL, MENU_FLAG, FS_CONCENTRATE, 0);
    menu_divider( ch, 6 );
    menu_option( ch, 7, "Desired Fighting Distance", (void **) &ch->des_dist, MENU_INT, ch->des_dist);
    do_menu(ch, "save", "Fighting Style");
}

void do_fightstyle(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (arg[0] == 0)
    {
	banner2_to_char("FIGHTING STYLE", ch);
	stc("\n\r",ch);
	if (IS_SET(ch->fight_style, FS_BLOCK))
	    sprintf(buf, "(BLOCK) You will block all attacks and will not attack back.");
	else if (IS_SET(ch->fight_style, FS_ATTACK))
	    sprintf(buf, "(ATTACK) You will not block attacks, only attack.");
	else if (IS_SET(ch->fight_style, FS_CONSERVATIVE))
	    sprintf(buf, "(CONSERVATIVE) You will only attack when you can block a counter attack.");
	else if (IS_SET(ch->fight_style, FS_AGGRESSIVE))
	    sprintf(buf, "(AGGRESSIVE) You will attack whenever possible and block all you can.");
	cent_to_char(buf, ch);
	if (IS_SET(ch->fight_style, FS_CONCENTRATE))
	    sprintf(buf, "(CONCENTRATE) You will concentrate all your attacks on main opponent.");
	else
	    sprintf(buf, "(SPREAD) You will spread attacks among all your opponents.");
	cent_to_char(buf, ch);
	sprintf(buf, "Your desired fighting distance is %d.", ch->des_dist);
	cent_to_char(buf, ch);
	if (IS_SET(ch->fight_style, FS_RETREAT))
	    cent_to_char("You are currently retreating from combat.", ch);
	stc("\n\r",ch);
	banner2_to_char("BLOCKING STYLE", ch);
	if (ch->block_arm == -1) cent_to_char("(NOPREF) You have no prefered blocking arm.",ch);
	if (ch->block_arm == 0) cent_to_char("(RIGHT) After your preference of your right arm:",ch);
	if (ch->block_arm == 1) cent_to_char("(LEFT) After your preference of your left arm:",ch);
	if (ch->block_arm == 2) cent_to_char("(THIRD) After your preference of your third arm:",ch);
	if (ch->block_arm == 3) cent_to_char("(FOURTH) After your preference of your fourth arm:",ch);
	sprintf(buf, "(HIGHEST) You will block with the arm with the highest initiative.");
	if (IS_SET(ch->fight_style, FS_LOWEST_BLOCK))
	    sprintf(buf, "(LOWEST) You will block with the arm with the lowest initiative.");
	else if (IS_SET(ch->fight_style, FS_ANY_BLOCK))
	    sprintf(buf, "(ANY) You will block with any available arm.");
    	else if (IS_SET(ch->fight_style, FS_SELECTIVE_BLOCK) && ch->block_arm > -1)
	    sprintf(buf, "(SELECTED) You will block with only your preferred arm.");
	cent_to_char(buf, ch);
	stc("\n\r",ch);
	banner2_to_char("OPTIONS", ch);
	stc("\n\r",ch);
	sprintf(buf, "Combat options:  distance");
	if (!IS_SET(ch->fight_style, FS_BLOCK))
	    strcat(buf, " block");
	if (!IS_SET(ch->fight_style, FS_ATTACK))
	    strcat(buf, " attack");
	if (!IS_SET(ch->fight_style, FS_CONSERVATIVE))
	    strcat(buf, " conservative");
	if (!IS_SET(ch->fight_style, FS_AGGRESSIVE))
	    strcat(buf, " aggressive");
	if (!IS_SET(ch->fight_style, FS_CONCENTRATE))
	    strcat(buf, " concentrate");
	else
	    strcat(buf, " spread");
	cent_to_char(buf, ch);
	sprintf(buf, "Blocking Options: ");
	if (ch->block_arm != -1)
	    strcat(buf, " nopref");
	if (ch->block_arm != 0)
	    strcat(buf, " right");
	if (ch->block_arm != 1)
	    strcat(buf, " left");
	if (ch->block_arm != 2)
	    strcat(buf, " third");
	if (ch->block_arm != 3)
	    strcat(buf, " fourth");
	if (!IS_SET(ch->fight_style, FS_HIGHEST_BLOCK))
	    strcat(buf, " highest");
	if (!IS_SET(ch->fight_style, FS_LOWEST_BLOCK))
	    strcat(buf, " lowest");
	if (!IS_SET(ch->fight_style, FS_ANY_BLOCK))
	    strcat(buf, " any");
	if (!IS_SET(ch->fight_style, FS_SELECTIVE_BLOCK))
	    strcat(buf, " selected");
	cent_to_char(buf, ch);
	stc("\n\r",ch);
	divide6_to_char( ch );
	return;
    }

    if (!str_prefix(arg, "highest") && !IS_SET(ch->fight_style, FS_HIGHEST_BLOCK))
    {
	SET_BIT(ch->fight_style, FS_HIGHEST_BLOCK);
	REMOVE_BIT(ch->fight_style, FS_LOWEST_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_ANY_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_SELECTIVE_BLOCK); 
	stc("You will now block with the arm with the highest initiative.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "lowest") && !IS_SET(ch->fight_style, FS_LOWEST_BLOCK))
    {
	REMOVE_BIT(ch->fight_style, FS_HIGHEST_BLOCK);
	SET_BIT(ch->fight_style, FS_LOWEST_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_ANY_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_SELECTIVE_BLOCK); 
	stc("You will now block with the arm with the lowest initiative.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "any") && !IS_SET(ch->fight_style, FS_ANY_BLOCK))
    {
	REMOVE_BIT(ch->fight_style, FS_HIGHEST_BLOCK);
	REMOVE_BIT(ch->fight_style, FS_LOWEST_BLOCK); 
	SET_BIT(ch->fight_style, FS_ANY_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_SELECTIVE_BLOCK); 
	stc("You will now block with any arm at random.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "selected") && !IS_SET(ch->fight_style, FS_SELECTIVE_BLOCK))
    {
	SET_BIT(ch->fight_style, FS_HIGHEST_BLOCK);
	REMOVE_BIT(ch->fight_style, FS_LOWEST_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_ANY_BLOCK); 
	REMOVE_BIT(ch->fight_style, FS_SELECTIVE_BLOCK); 
	stc("You will now block with only your prefferred arm.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "nopref") && ch->block_arm != -1)
    {
	ch->block_arm = -1;
	stc("You now have have no preffered blocking arm.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "right") && ch->block_arm != 0)
    {
	ch->block_arm = 0;
	stc("Your right arm is now your preffered blocking arm.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "left") && ch->block_arm != 1)
    {
	ch->block_arm = 1;
	stc("Your left arm is now your preffered blocking arm.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "third") && ch->block_arm != 2)
    {
	ch->block_arm = 2;
	stc("Your third arm is now your preffered blocking arm.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "fouth") && ch->block_arm != 3)
    {
	ch->block_arm = 3;
	stc("Your fourth arm is now your preffered blocking arm.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "distance"))
    {
	int val;

	if (argument[0] == 0 || (val = atoi(argument)) < 0 || val > MAX_DIST  )
	{
	    stcf(ch, "Syntax:  fightstyle distance <number>\n\r\nWhere <number> is your desired fighting distance in the range of 0 to %d\n\r", MAX_DIST);
	    return;
	}

	ch->des_dist = val;
	sprintf(buf, "Desired fighting distance set to %d.\n\r", val);
	stc(buf, ch);
	return;
    }

    if (!str_prefix(arg, "block") && !IS_SET(ch->fight_style, FS_BLOCK))
    {
	SET_BIT(ch->fight_style, FS_BLOCK);
	REMOVE_BIT(ch->fight_style, FS_ATTACK);
	REMOVE_BIT(ch->fight_style, FS_CONSERVATIVE);
	REMOVE_BIT(ch->fight_style, FS_AGGRESSIVE);
	stc("You will now block all attacks without counter attacking.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "attack") && !IS_SET(ch->fight_style, FS_ATTACK))
    {
	REMOVE_BIT(ch->fight_style, FS_BLOCK);
	SET_BIT(ch->fight_style, FS_ATTACK);
	REMOVE_BIT(ch->fight_style, FS_CONSERVATIVE);
	REMOVE_BIT(ch->fight_style, FS_AGGRESSIVE);
	stc("You will now attack whenever possible, without blocking.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "conservative") && !IS_SET(ch->fight_style, FS_CONSERVATIVE))
    {
	REMOVE_BIT(ch->fight_style, FS_BLOCK);
	REMOVE_BIT(ch->fight_style, FS_ATTACK);
	SET_BIT(ch->fight_style, FS_CONSERVATIVE);
	REMOVE_BIT(ch->fight_style, FS_AGGRESSIVE);
	stc("You will now attack only when you can block a counter attack.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "aggressive") && !IS_SET(ch->fight_style, FS_AGGRESSIVE))
    {
	REMOVE_BIT(ch->fight_style, FS_BLOCK);
	REMOVE_BIT(ch->fight_style, FS_ATTACK);
	REMOVE_BIT(ch->fight_style, FS_CONSERVATIVE);
	SET_BIT(ch->fight_style, FS_AGGRESSIVE);
	stc("You will now attack and block whenever possible.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "concentrate") && !IS_SET(ch->fight_style, FS_CONCENTRATE))
    {
	SET_BIT(ch->fight_style, FS_CONCENTRATE);
	stc("You will now concentrate all attacks on your main opponent.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "spread") && IS_SET(ch->fight_style, FS_CONCENTRATE))
    {
	REMOVE_BIT(ch->fight_style, FS_CONCENTRATE);
	stc("You will now spread your attacks among all your opponents.\n\r",ch);
	return;
    }

    do_fightstyle(ch, "");    
}


void check_group_combat(CHAR_DATA *ch, CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    crash("checking group combat");

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
 	if (rch->altitude != ch->altitude) continue;

        crash("checking group combat 2");
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{
	    if (!IS_NPC(ch) && IS_NPC(rch)
		&& IS_SET(rch->off_flags, ASSIST_PLAYERS))
	    {
		do_emote(rch, "screams and attacks!");
		set_fighting(rch, victim);
                crash("checking group combat 3");
		char_attack(rch);
		continue;
	    }
            crash("checking group combat 3");

	    if(!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act, PLR_AUTOASSIST))
		|| IS_AFFECTED(rch, AFF_CHARM))
		&& is_same_group(ch, rch)
		&& !is_safe(rch, victim))
		set_fighting(rch, victim);
		char_attack(rch);
		continue;
	    }

	    if (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM))
	    {
                if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))         
                                                                                
                ||   (IS_NPC(rch) && rch->group && rch->group ==ch->group)     
                                                                                
                ||   (IS_NPC(rch) && rch->race == ch->race                      
                   && IS_SET(rch->off_flags,ASSIST_RACE))                       
                                                                                
                ||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)        
                   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))                       
                     ||  (IS_EVIL(rch)    && IS_EVIL(ch))))
                                                                                
               ||   (rch->pIndexData == ch->pIndexData                         
                   && IS_SET(rch->off_flags,ASSIST_VNUM)))
		{
                    CHAR_DATA *vch;                                             
                    CHAR_DATA *target;                                          
                    int number;                                                 
                                                                                
                    if (number_bits(1) == 0)                                    
                        continue;                                               
                    crash("checking group combat 3");
                                                                                
                    target = NULL;                                              
                    number = 0;                                                 
                    for (vch = ch->in_room->people; vch; vch = vch->next)       
                    {                                                           
                        if (can_see(rch,vch)                                    
                        &&  is_same_group(vch,victim)                           
                        &&  number_range(0,number) == 0)                        
                        {                                                       
                            target = vch;                                       
                            number++;                                           
                        }                                                       
                    }                                                           
                                                                                
	     	    crash("checking group combat 8");
                    if (target != NULL)                                         
                    {                                                           
                        do_emote(rch,"screams a war cry and joins the fray!");  
			set_fighting(rch, target);
                        char_attack(rch);                   
		    }
		}
	    }
	}
    }
}

int misc_body_combat_penalty(CHAR_DATA *ch, int chance)
{
    int chmod = 100;

    crash("misc_body_combat_penalty");

    if (BODY_PART(ch, BODY_RIGHT_EAR))
	chmod -= 5;
    if (BODY_PART(ch, BODY_LEFT_EAR))
	chmod -= 5;

    if (BODY_PART(ch, BODY_LEFT_EYE))
	chmod -= 10;

    if (BODY_PART(ch, BODY_RIGHT_EYE))
	chmod -= 10;

    if (BODY_PART(ch, BODY_LEFT_EYE)
     && BODY_PART(ch, BODY_RIGHT_EYE))
	chmod -= 70;

    if (BODY_PART(ch, BODY_LEFT_LEG))
	chmod -= 10;

    if (BODY_PART(ch, BODY_RIGHT_LEG))
	chmod -= 10;

    if (BODY_PART(ch, BODY_LEFT_LEG)
     && BODY_PART(ch, BODY_RIGHT_LEG))
	chmod -= 70;

    crash("misc_body_combat_penalty place 2");

    if (chmod <= 0) return 0;

    return ((chance * 100) / chmod);
}

int calc_initiative_gain( CHAR_DATA *ch )
{
    int gain = 5;

    crash("Calcing initiave gain");

    if (ch->hit < 1) return 0;

    if (ch->power[DISC_DRAG_META] > 0)
	gain += ch->power[DISC_DRAG_META];

    return gain / 2;
}

int calc_max_initiative( CHAR_DATA *ch )
{
    return calc_initiative_gain( ch ) * 12;
}

int initiative_cost( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int cost = 0;

    crash("initiave cost place 1");

    if (obj == NULL || obj->item_type != ITEM_WEAPON)
    {
	cost = (get_size(ch) + 4);
	if (get_extra_attacks(ch) < 0) cost++;
	else if (get_extra_attacks(ch) >= 1) cost--;

	if (get_extra_attacks(ch) >= 3) cost--;
	if (get_extra_attacks(ch) >= 5) cost--;

	if (cost < 4) cost = 4;

        crash("initiave cost place 2");

	if (ch->stance == 7)
	    cost = cost * (100 - ch->stance_skill[7] / 2) / 100;

	return cost;
    }

//   if (IS_WEAPON_STAT(obj, WEAPON_RUNE_DANCING) )
//   {
//	if (dice(1, 6) == 1)
//	{
//	    stc("The dancing sigil flares and vanishes.\n\r", ch);
//	    REMOVE_BIT(obj->value[4], WEAPON_RUNE_DANCING);
//	}
//	cost -= 3;
//    }

    if (obj->value[6] > 0)
	cost += obj->value[6];

    else if (obj->value[0] == WEAPON_SWORD) cost += 8;
    else if (obj->value[0] == WEAPON_POLEARM) cost += 14;
    else if (obj->value[0] == WEAPON_FLAIL) cost += 8;
    else if (obj->value[0] == WEAPON_DAGGER) cost += 6;
    else if (obj->value[0] == WEAPON_WHIP) cost += 14;
    else if (obj->value[0] == WEAPON_MACE) cost += 8;
    else if (obj->value[0] == WEAPON_SPEAR) cost += 12;
    else if (obj->value[0] == WEAPON_AXE) cost += 10;
    else cost += 8;

    /* Now cap all the costs with the extra attacks */
    cost -= get_extra_attacks(ch);
    if (cost < 0) cost = 0;

    cost += get_size(ch);

    if (cost < 4) cost = 4;
    if (ch->stance == 7)
	cost = cost * (100 - ch->stance_skill[7] / 2) / 100;
    return cost;
}

int attack_reach( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int re = get_size(ch) * 2;

    crash("attack_reach procedure");

    if (ch->secondaryrace == RACE_DRAGON)
	re += 2;

    if (get_size(ch) > SIZE_LARGE) re += (get_size(ch) - SIZE_LARGE) * 2;

    if (IS_NPC(ch))
    {
	if (re > 4) re += 4;
	else re += 2;
    }

    if (obj == NULL || obj->item_type != ITEM_WEAPON)
    {
	return re;
    }

    re -= 2;
    if (re < 0) re = 0;

    if (obj->value[7] > 0)
	return obj->value[7] + re;

    crash("attack_reach procedure place 2");

    if (obj->value[0] == WEAPON_SWORD) return 6 + re;
    if (obj->value[0] == WEAPON_POLEARM) return 10 + re;
    if (obj->value[0] == WEAPON_FLAIL) return 4 + re;
    if (obj->value[0] == WEAPON_DAGGER) return 2 + re;
    if (obj->value[0] == WEAPON_WHIP) return 8 + re;
    if (obj->value[0] == WEAPON_MACE) return 4 + re;
    if (obj->value[0] == WEAPON_SPEAR) return 8 + re;
    if (obj->value[0] == WEAPON_AXE) return 5 + re;
    return 5 + re;
}

int des_dist( CHAR_DATA *ch )
{
    crash("Des dist");

    if (IS_NPC(ch) && ch->des_dist == 0)
    {
	int min, max, d;

	min = attack_reach( ch, get_eq_char(ch, WEAR_RIGHT_HAND) );
	max = min;
	d = attack_reach( ch, get_eq_char(ch, WEAR_LEFT_HAND) );
	if (d < min) min = d;
	if (d > max) max = d;
	if (dice(1, 2) == 1) ch->des_dist = min; else ch->des_dist = max;
    }
    return ch->des_dist;
}

void create_stun( CHAR_DATA *ch, int stun )
{
    if (ch->secondaryrace == RACE_VAMPIRE)
	stun /= 2;

    stun -= ch->mod_stat[STAT_CON] / 15;

    if (stun < 1)
	stun = 1;

    ch->stunned += dice(1, stun);

    if (ch->stunned > 10)
	ch->stunned = 10;
}

void stunning_hit( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int stun;

    stun = 4 + get_size(ch) - get_size(victim);

    if (stun > 0) {
	act("$N appears to be stunned by the sheer force of $n's blow.", ch, NULL, victim, TO_NOTVICT);
	act("The impact of $n's blow stuns you.", ch, NULL, victim, TO_VICT);
	act("$N is stunned by the power of your blow.", ch, NULL, victim, TO_CHAR);
	create_stun(victim, stun);
    }
}

int advance_rate( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *hand;
    int rate = 0;
    int chance = 30;
    int trip_chance = 0;

    if (ch->stance == 9)
	chance += ch->stance_skill[9] / 2;

    crash("advance rate");

    if (ch->hit < 1) return 0;

    if ( ch->distance < des_dist(ch) )
	rate = 1;
    else if ( ch->distance > des_dist(ch) )
	rate = -1;

    if (rate == 0 || ( ch->distance < des_dist(victim) && rate == 1 )
	|| ( ch->distance > des_dist(victim) && rate == -1 ))
	return rate;


    trip_chance = 10 + ch->stance_skill[ch->stance] / 8;
    if (ch->stance == 9) trip_chance += ch->stance_skill[9];
    if (is_flying(ch)) trip_chance = 0;

    crash("advance_rate procedure place 2");

    if (rate == 1 && dice(1, 3) == 1)
    {
	if (dice(1, trip_chance) == 1 && get_size(ch) < SIZE_HUGE)
	{
	    act("$n trips over while trying to advance on $N.", ch, NULL, victim, TO_NOTVICT);
	    act("You trip over while trying to advance on $N.", ch, NULL, victim, TO_CHAR);
	    act("$n trips over while trying to advance on you.", ch, NULL, victim, TO_VICT);
	    ch->initiative[0] = 0;
	    ch->initiative[1] = 0;
	    ch->initiative[2] = 0;
	    ch->initiative[3] = 0;
	    ch->initiative[4] = 0;
	    create_stun(ch, 4);
	    ch->distance = victim->des_dist;
	    victim->distance = victim->des_dist;
            crash("advance rate 2");
	    return 0;
	}
        crash("advance rate 3");
	return rate;
    }

    if (rate == -1 && dice(1, 3) == 1)
    {
	if (dice(1, trip_chance) == 1 && get_size(ch) < SIZE_HUGE)
	{
	    act("$n trips over while trying to back away from $N.", ch, NULL, victim, TO_NOTVICT);
	    act("You trip over while trying to back away from $N.", ch, NULL, victim, TO_CHAR);
	    act("$n trips over while trying to back away from you.", ch, NULL, victim, TO_VICT);
	    ch->initiative[0] = 0;
	    ch->initiative[1] = 0;
	    ch->initiative[2] = 0;
	    ch->initiative[3] = 0;
	    ch->initiative[4] = 0;
	    ch->distance = victim->des_dist;
            crash("advance rate 2");
	    return 0;
	}
        crash("advance rate 3.5");
	return rate;
    }

    if (dice(1, 5) != 3) return FALSE;

    chance += get_skill( ch, gsn_dodge );
    check_improve(ch, gsn_dodge, TRUE, 5);

    crash("advance rate 4");

    if (!BODY_PART(victim, BODY_RIGHT_ARM) && !BODY_PART(victim, BODY_RIGHT_HAND) && victim->initiative[0] > 5)
    {
	hand = get_eq_char( victim, WEAR_RIGHT_HAND );
        crash("advance rate 5");
	if (hand != NULL && hand->item_type == ITEM_WEAPON)
	{
	    if (ch->distance <= attack_reach( victim, hand ) && (ch->distance + 3) > attack_reach(victim, hand))
	        chance -= get_skill(victim, get_weapon_sn_by_type( hand )) / 3;
	}
	else if (ch->distance <= attack_reach( victim, NULL ))
		chance -= get_skill(victim, gsn_hand_to_hand) / 5;
    }

    if (!BODY_PART(victim, BODY_LEFT_ARM) && !BODY_PART(victim, BODY_LEFT_HAND) && victim->initiative[0] > 5)
    {
	hand = get_eq_char( victim, WEAR_LEFT_HAND );
        crash("advance rate 6");
	if (hand != NULL && hand->item_type == ITEM_WEAPON)
	{
	    if (ch->distance <= attack_reach( victim, hand ) && (ch->distance + 3) > attack_reach(victim, hand))
	        chance -= get_skill(victim, get_weapon_sn_by_type( hand )) / 3;
	}
	else if (ch->distance <= attack_reach( victim, NULL ))
		chance -= get_skill(victim, gsn_hand_to_hand) / 5;
    }

    if (BODY_PART(victim, BODY_THIRD_ARM) && BODY_PART(victim, BODY_THIRD_HAND) && victim->initiative[0] > 5)
    {
	hand = get_eq_char( victim, WEAR_THIRD_HAND );
        crash("advance rate 7");
	if (hand != NULL && hand->item_type == ITEM_WEAPON)
	{
	    if (ch->distance <= attack_reach( victim, hand ) && (ch->distance + 3) > attack_reach(victim, hand))
	        chance -= get_skill(victim, get_weapon_sn_by_type( hand )) / 3;
	}
	else if (ch->distance <= attack_reach( victim, NULL ))
		chance -= get_skill(victim, gsn_hand_to_hand) / 5;
    }

    if (BODY_PART(victim, BODY_FOURTH_ARM) && BODY_PART(victim, BODY_FOURTH_HAND) && victim->initiative[0] > 5)
    {
	hand = get_eq_char( victim, WEAR_FOURTH_HAND );
        crash("advance rate 8");
	if (hand != NULL && hand->item_type == ITEM_WEAPON)
	{
	    if (ch->distance <= attack_reach( victim, hand ) && (ch->distance + 3) > attack_reach(victim, hand))
	        chance -= get_skill(victim, get_weapon_sn_by_type( hand )) / 3;
	}
	else if (ch->distance <= attack_reach( victim, NULL ))
		chance -= get_skill(victim, gsn_hand_to_hand) / 5;
    }

    crash("advance rate 9");

    if (dice(1, 100) < chance) return rate;

    if (rate == -1)
    {
	act("$n tries to advance on $N but is held back.", ch, NULL, victim, TO_NOTVICT);
	act("You try to advance on $N but $E fights you back.", ch, NULL, victim, TO_CHAR);
	act("$n tries to advance on you, but you fight $M back.", ch, NULL, victim, TO_VICT);
    }

    return 0;
}

bool will_attack( CHAR_DATA *ch, int cur_init, int init_cost )
{
    crash("will attack 1");
    if (IS_FLAG6(ch, AFF6_AUTO_ATTACK) && cur_init > 1)
	return TRUE;

    if (IS_SET(ch->fight_style, FS_BLOCK) || IS_SET(ch->fight_style, FS_RETREAT))
	return FALSE;

    if (IS_SET(ch->fight_style, FS_ATTACK) || IS_SET(ch->fight_style, FS_AGGRESSIVE))
    {
	if (cur_init >= init_cost)
	    return TRUE;
	else return FALSE;
    }

    if (cur_init >= init_cost + 3)
	return TRUE;

    crash("will attack 2");
    return FALSE;
}

void check_funky_weapon(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield)
{
    crash("Damn funky weapons!");

    /* but do we have a funky weapon? 
	or just a FREAKY one?   btw, this bit works - i tested! */ 

    /* This bit is for age attacks... ghosts etc.... */
    if (wield == NULL && IS_FLAG5(ch, AFF5_AGE_ATTACK) && dice(1, 6) == 1)
    {
	stc("You feel your life draining away.\n\r", victim);
	if (!IS_NPC(victim))
	{
	    victim->pcdata->extra_age++;
	    stc("You are one year older.\n\r", victim);
 	}
    }


    if (ch == NULL || victim == NULL || wield == NULL) return;
    if (ch->fighting != victim) return;

    if (wield != NULL)  
    {
        int dam;

        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
        {
            int level;
            AFFECT_DATA *poison, af;
                
            if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
                level = wield->level;
            else
                level = poison->level;
             
            if (!saves_spell(level / 2,victim,DAM_POISON))
            {
                send_to_char("You feel poison coursing through your veins.",
                    victim);
                act("$n is poisoned by the venom on $p.",
                    victim,wield,NULL,TO_ROOM);

                af.where     = TO_AFFECTS;
                af.type      = gsn_poison; 
                af.level     = level * 3/4;
                af.duration  = level / 2;
                af.location  = APPLY_STR;
                af.modifier  = -1;
                af.bitvector = AFF_POISON;
                affect_join( victim, &af );
            }
         
            /* weaken the poison if it's temporary */
            if (poison != NULL)
            {   
                poison->level = UMAX(0,poison->level - 2);
                poison->duration = UMAX(0,poison->duration - 1);

                if (poison->level == 0 || poison->duration == 0)
                    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
            }
        }
                    
        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
        {
            dam = number_range(1, wield->level / 5 + 1);
            act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
            act("You feel $p drawing your life away.",
                victim,wield,NULL,TO_CHAR);
            damage_old(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->alignment = UMAX(-1000,ch->alignment - 1);
            ch->hit += dam/2;
	}

        if (ch->fighting == victim && (IS_WEAPON_STAT(wield,WEAPON_FLAMING)
                || IS_WEAPON_STAT(wield, WEAPON_ELE_FLAME) )  )
        {
            dam = number_range(1,wield->level / 4 + 1);
            act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
            act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
            fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_FIRE,FALSE);
        }

	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_SMITE_ELADRIN)
	    && victim->secondaryrace == RACE_ELADRIN)
  	{
	    act("$n is smited by the power within $p.", victim, wield, NULL, TO_ROOM);
	    act("You are smited by the power within $p.", victim, wield, NULL, TO_CHAR);
	    damage(ch, victim, 40, 0, DAM_OTHER, FALSE);
	}
                    
	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_SMITE_WARLOCKS)
	    && victim->secondaryrace == RACE_WARLOCK)
  	{
	    act("$n is smited by the power within $p.", victim, wield, NULL, TO_ROOM);
	    act("You are smited by the power within $p.", victim, wield, NULL, TO_CHAR);
	    damage(ch, victim, 40, 0, DAM_OTHER, FALSE);
	}
                    
	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_SMITE_DEMONS)
	    && victim->secondaryrace == RACE_DAEMON)
  	{
	    act("$n is smited by the power within $p.", victim, wield, NULL, TO_ROOM);
	    act("You are smited by the power within $p.", victim, wield, NULL, TO_CHAR);
	    damage(ch, victim, 40, 0, DAM_OTHER, FALSE);
	}
                    
	if (ch->fighting == victim
	    && IS_WEAPON_STAT(wield, WEAPON_SMITE_DRAGONS)
	    && victim->secondaryrace == RACE_DRAGON)
  	{
	    act("$n is smited by the power within $p.", victim, wield, NULL, TO_ROOM);
	    act("You are smited by the power within $p.", victim, wield, NULL, TO_CHAR);
	    damage(ch, victim, 40, 0, DAM_OTHER, FALSE);
	}
                    
        if (ch->fighting == victim && (IS_WEAPON_STAT(wield,WEAPON_FROST)
                || IS_WEAPON_STAT(wield, WEAPON_ELE_WATER) ) )
        {   
            dam = number_range(1,wield->level / 6 + 2);
            act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
            act("The cold touch of $p surrounds you with ice.",
                victim,wield,NULL,TO_CHAR);
            cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_COLD,FALSE);   
        }
            
        if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_ELE_EARTH) )
        {
            dam = number_range(1,wield->level / 5 + 2);
            act("$p covers $n in acid.",victim,wield,NULL,TO_ROOM);
            act("The corrosive touch of $p surrounds you with acid.",
                victim,wield,NULL,TO_CHAR);
            acid_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_COLD,FALSE);
        }
            
        if (ch->fighting == victim && (IS_WEAPON_STAT(wield,WEAPON_SHOCKING)
                || IS_WEAPON_STAT(wield, WEAPON_ELE_AIR) ) )
        {
            dam = number_range(1,wield->level/5 + 2);
            act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
            act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
            shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
            damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
        }
            
        if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_SUNBLADE))
        {
            dam = number_range(wield->level/2, wield->level+1);
            if (victim->alignment > -100)
                dam = 0;
            if (victim->alignment > -300)
 
            if (victim->alignment > -300)
                dam = dam / 2;
            if (victim->alignment == -1000)
                dam *= 2;
            if (victim->secondaryrace == RACE_VAMPIRE)
                dam *= 4;
            if (dam > 0)
            {
                act("$n's flesh is seared by the radiant aura of $p.",victim,wield,NULL,TO_ROOM);
                act("Your flesh is seared by the radiant aura of $p.",victim,wield,NULL,TO_CHAR);
                damage(ch,victim,dam,0,DAM_LIGHT,FALSE);
                ch->alignment = UMAX(-1000,ch->alignment + 1);  
	    }
        }
            
        if (ch->fighting == victim
                && IS_WEAPON_STAT(wield, WEAPON_DRAGONLANCE))   
        {
            dam = number_range(wield->level/2, wield->level+1)*4;
            if (victim->secondaryrace != RACE_DRAGON)
                dam = 0;
            if (dam > 0)
            {
            act("$n's flesh is torn apart by $p.",victim,wield,NULL,TO_ROOM);
            act("Your flesh is torn apart by $p.",victim,wield,NULL,TO_CHAR);
            damage(ch,victim,dam,0,DAM_LIGHT,FALSE);
            }
        }
                
        if (ch->fighting == victim
                && IS_WEAPON_STAT(wield, WEAPON_RUNE_FORCE_BOLT) )
        {
            dam = dice(ch->power[DISC_HERO_SIGI], 5);
            act("A bolt of energy leaps from $p to $n.", victim, wield, NULL, TO_ROOM);
            act("A bolt of energy leaps from $p to you.", victim, wield, NULL, TO_CHAR);
            damage (ch, victim, dam, 0, DAM_ENERGY, FALSE);
            if(dice(1, 18) > ch->power[DISC_HERO_SIGI] * 3)
            {
                send_to_char("The force bolt sigil flares and vanishes.\n\r", ch);
                REMOVE_BIT(wield->value[4],WEAPON_RUNE_FORCE_BOLT);
            }
        }
            
        if (ch->fighting == victim
                && IS_WEAPON_STAT(wield, WEAPON_RUNE_SMITE_EVIL) )
        {
            dam = 0 - (victim->alignment / 6);
            if (dam <= 0) dam = 0;
             act("White ethereal vapours flow from $p into $n.", victim, wield, NULL, TO_ROOM);
            act("White ethereal vapours from $p to you.", victim, wield, NULL, TO_CHAR);
            damage(ch, victim, dam, 0, DAM_HOLY, FALSE);
            if(dice(1, 18) > ch->power[DISC_HERO_SIGI] * 3)
            {
                send_to_char("The smite evil sigil flares and vanishes.\n\r", ch);
                REMOVE_BIT(wield->value[4],WEAPON_RUNE_SMITE_EVIL);
            }
        }
             
        if (ch->fighting == victim
                && IS_WEAPON_STAT(wield, WEAPON_RUNE_BLAZE) )
        {   
            dam = ch->power[DISC_HERO_SIGI] * ch->generation;
            act("A gout of flame shoots out of $p into $n.", victim, wield, NULL, TO_ROOM);
            act("A huge gout of flame shoots out of $p into you.", victim, wield, NULL, TO_CHAR);
            damage(ch, victim, dam, 0, DAM_FIRE, FALSE);
            if(dice(1, 18) > ch->power[DISC_HERO_SIGI] * 3)   
            {
                send_to_char("The blaze sigil flares and vanishes.\n\r", ch);
                REMOVE_BIT(wield->value[4],WEAPON_RUNE_BLAZE);
            }
        }
        if (ch->fighting == victim
                && IS_WEAPON_STAT(wield, WEAPON_RUNE_LIGHTNING) )
        {
            dam = ch->power[DISC_HERO_SIGI] * ch->generation * 3 / 2;
            act("A bolt of lightning shoots out of $p at $n.", victim, wield, NULL, TO_ROOM);
            act("A bolt of lightning shoots out of $p into you.", victim, wield, NULL, TO_CHAR);
            damage(ch, victim, dam, 0, DAM_LIGHTNING, FALSE);
            if(dice(1, 18) > ch->power[DISC_HERO_SIGI] * 3)
            {
                send_to_char("The lightning sigil flares and vanishes.\n\r", ch);
                REMOVE_BIT(wield->value[4],WEAPON_RUNE_LIGHTNING);
            }
        }
            
        if (ch->fighting == victim && IS_WEAPON_STAT(wield, WEAPON_SILVER))
        {
            dam = number_range(wield->level/2, wield->level+1)*4;
            if (victim->secondaryrace == RACE_LUPINE)
                dam = (dam * (100 - victim->value[0]) / 100 );
            if (victim->secondaryrace != RACE_LUPINE)
                dam = 0;
            if (dam > 0)
            {
                act("$n's flesh is torn apart by $p.",victim,wield,NULL,TO_ROOM);
                act("Your flesh is torn apart by $p.",victim,wield,NULL,TO_CHAR);
                damage(ch,victim,dam,0,DAM_LIGHT,FALSE);
            }
        }
    }
}

void perform_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *hand, bool blockable)
{
    int dam_type, dt, sn, dam = 0, didhit;
    int diceroll, stmod, attack_modify;
    int stun = 200;

    if (ch->stance == 8)
        stun = stun * (100 - ch->stance_skill[ch->stance] / 2) / 100;
    if (victim->stance == 8)
        stun = stun * (100 + ch->stance_skill[ch->stance] / 2) / 100;
        
    if (IS_NPC(victim) && victim->level > 100)
        stun += victim->level - 50;
        
    if (IS_NPC(ch) && ch->level > 100)
        stun -= ch->level - 50;

    if (hand != NULL
        && hand->item_type == ITEM_WEAPON)
    {
        dt = TYPE_HIT + hand->value[3];
        if (dt < TYPE_HIT)
            dam_type = attack_table[hand->value[3]].damage;
        else
            dam_type = attack_table[dt - TYPE_HIT].damage;
        if (dam_type == -1)
            dam_type = DAM_BASH;
        sn = get_weapon_sn_by_type(hand);
        crash("perform hit 1");
        if ((didhit = did_hit(ch, victim, sn, dam_type)) > 0)
        {
	    /* hit - calc dodges, parriess etc. */
	    if (blockable && (check_parry(ch, victim, didhit)
	        || check_dodge(ch, victim)
	        || check_shield_block(ch, victim)) && didhit != 2)
	    {
	        /* blocked */
	    }
	    else
	    {
	        if (sn != -1)
                    check_improve(ch, sn, FALSE, 5);
                if (hand != NULL)
                    dam = dice(hand->value[1], hand->value[2]) * WEAPON_DAM * get_weapon_skill(ch, sn) / 100;
                if (IS_WEAPON_STAT(hand, WEAPON_SHARP))
                {
                    int percent;
                    if ((percent = number_percent()) <= (get_weapon_skill(ch, sn) / 8))
                        dam = 2 * dam + (dam * 2 * percent / 100);
                }

                crash("perform hit 2");
                if (get_skill(ch, gsn_enhanced_damage) > 0)
                {
                    diceroll = number_percent();
                    if (diceroll <= get_skill(ch,gsn_enhanced_damage))
                    {
                        check_improve(ch,gsn_enhanced_damage,TRUE,6);
                        dam += 2 * (dam * diceroll / 300);
                    }
                }

                if (stances[ch->stance].score[victim->stance] >= 0)
                    stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
                else
                {
                    stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
                    if (stmod < 10) stmod = 10;
                }

                if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
                {
                    if (dice(1, 2) == 1)
                    improve_stance(ch, ch->stance);
                }

                dam = (dam * stmod) / 100;

                if (!IS_AWAKE(victim))
                    dam *= 4;
                else if (victim->position < POS_FIGHTING)
                    dam *= 2;

                dam += GET_DAMROLL(ch) * UMIN(100,get_weapon_skill(ch, sn)) / 100;

                if (ch->power[DISC_DAEM_GELU] > 1)
                    dam += ch->played / 18000;

                if (ch->power[DISC_HERO_BATT] > 1)
                    dam += ch->played / 10000;

                if (didhit == 2) dam = dam * 3 / 2;

                dam = cap_dam(ch, victim, dam, dam_type, TRUE);
                crash("called cap dam place 2");
                attack_modify = dice(1, 100);  
                dam = randomize_damage( ch, dam, attack_modify );
                display_weapon_damage(ch, victim, hand, dam, attack_modify);

                crash("perform hit 3"); 
                check_funky_weapon(ch, victim, hand);
                if (ch == NULL || victim == NULL) return;
                if (ch->fighting != victim) return;

                if (dam > stun && victim->distance < MAX_DIST)
                {
                    ch->distance += dam / 100;
                    if (ch->distance > MAX_DIST) ch->distance = MAX_DIST;
                    victim->distance = ch->distance;
                    stc("You are thrown backwards by the force of the blow.\n\r", victim);
                    act("$n is thrown back by the force of the blow.", victim, NULL, NULL, TO_ROOM);
                    stunning_hit(ch, victim);
                }

                if (deal_damage(ch, victim, dam)) return;
                if (dice(1, dam) > stun / 4)
                    create_stun(victim, 2);
                }
        }
        else
        {
            /* miss - display a cool random message */
            display_weapon_miss(ch,victim,hand);
        }
    }
    else if (hand == NULL)
    {               
        dam = 0;
        sn = gsn_hand_to_hand;
        dt = TYPE_HIT + ch->dam_type;
        if (dt < TYPE_HIT)
            dam_type = attack_table[ch->dam_type].damage;
        else
            dam_type = attack_table[dt - TYPE_HIT].damage;
        if (dam_type == -1)   
            dam_type = DAM_BASH;
        dam = get_punch_dam(ch);
        crash("perform hit 4");
        if (IS_NPC(ch))
            dam = dice( ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE] );
        if (IS_SET(ch->flag2, AFF_CLAW))
        {
            dam_type = DAM_SLASH;
        }
        else if (ch->cur_form == FRM_CRIMSON)
        {
            dam_type = DAM_NEGATIVE;
            dam = dam * 3 / 2;
        }
        else if (ch->cur_form == FRM_WHIRLWIND)
        {
            dam_type = DAM_BASH;
            dam = dam * 6 / 5;
        }
        else if (ch->cur_form == FRM_FLAME)
        {
            dam_type = DAM_FIRE;
            dam = dam * 3 / 2;
        }
        else if (ch->cur_form == FRM_LIGHT)
        {   
            dam_type = DAM_LIGHTNING;
            dam = dam * 5 / 4;
        }
                
        if (get_skill(ch, gsn_enhanced_damage) > 0)
        {
            diceroll = number_percent();
            if (diceroll <= get_skill(ch, gsn_enhanced_damage))
            {
                check_improve(ch, gsn_enhanced_damage,TRUE,6);
                dam += 2 * (dam * diceroll / 300);           
            }
        }
        crash("perform hit 5");

        if (stances[ch->stance].score[victim->stance] >= 0)
            stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
        else
        {
            stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
            if (stmod < 10) stmod = 10;
        }
        if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
        {
            if (dice(1, 2) == 1)
                improve_stance(ch, ch->stance);
        }
        dam = (dam * stmod) / 100;
                    
        dam += GET_DAMROLL(ch) / 5;
                 
        if ((didhit=did_hit(ch, victim, sn, dam_type)) > 0)
        {
            if (didhit == 19) dam *= 2;
            dam = cap_dam(ch, victim, dam, dam_type, TRUE);
            crash("called cap dam place 3");
            attack_modify = dice(1, 100);
            dam = randomize_damage( ch, dam, attack_modify ); 
            display_punch_damage(ch, victim, dam);
                     
            if (dam > stun && victim->distance < MAX_DIST)
            {
                ch->distance += dam / 100;
                if (ch->distance > MAX_DIST) ch->distance = MAX_DIST;
                victim->distance = ch->distance;
                stc("You are thrown backwards by the force of the blow.\n\r", victim);
                act("$n is thrown back by the force of the blow.", victim, NULL, NULL, TO_ROOM);
                stunning_hit(ch, victim);
            }

            if (deal_damage(ch, victim, dam)) return;
                     
            /* Lycanthropy */
            if (ch->secondaryrace == RACE_LUPINE   
                && victim->secondaryrace == 0
                && dice (1, 4000) < dam
                && victim->race != RACE_GNOME
                && IS_SET(ch->flag2, AFF_CLAW)  )
            {
                send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
                SET_BIT(victim->flag3, AFF_LYCANTHROPY);
            }
        }
        else
        {
            display_punch_miss(ch, victim);
        }
    }        
}

void char_attack( CHAR_DATA *ch )
{
    int initiative_gain = calc_initiative_gain( ch );
    int max_initiative = calc_max_initiative( ch );
    OBJ_DATA *hand;
    int dam_type, dt, sn, dam = 0, didhit;
    int chance, diceroll, stmod, attack_modify;
    CHAR_DATA *victim = ch->fighting;
    int rate;

    if (ch == NULL || victim == NULL || ch == victim)
	return;


    if (IS_NPC(ch) && ch->stance < 1 && ch->level >= 5)
    {
	if (ch->level < 20)
	{
	    set_stance(ch, dice(1, 5));
	    ch->stance_skill[ch->stance] = ch->level * 5;
	}
	else
	{
	    set_stance(ch, dice(1, 5) + 5);
	    ch->stance_skill[ch->stance] = (ch->level - 20) * 3 + 40;
	}
	if (ch->stance_skill[ch->stance] > 100)
	    ch->stance_skill[ch->stance] = 100;
    }
	    

    crash("char attack 1");

    if (ch->stunned > 0)
    {
	int i;

	ch->stunned -= dice(1, 2);
	for (i = 0; i < 5; i++)
	    ch->initiative[i] = 0;

	return;	
    }

    crash("char attack 2");

    ch->move -= 1;
    if (dice(1, ch->max_move) == 1 && !IS_NPC(ch))
    {
	ch->max_move += 1;
	ch->move += 1;
	ch->pcdata->perm_move += 1;
	stc("You feel fitter.\n\r", ch);
    }
    if (ch->move < 0)
    {
	int i;

	for (i = 0; i < 5; i++)
	    ch->initiative[i] = 0;

	ch->move = 0;
	return;
    }

    if (is_safe(ch, victim))
    {
	stop_fighting(victim, TRUE);
	return;
    }

    if (victim->riding != NULL)
    {
	if (dice(1, get_size(victim) + get_size(victim->riding)+2) >= get_size(victim) + 1)
	    victim = victim->riding;
    }
    else if (victim->riden_by != NULL)
    {
	if (dice(1, get_size(victim) + get_size(victim->riden_by)+2) >= get_size(victim) + 1)
	    victim = victim->riden_by;
    }

    crash("char attack 3");

    if (IS_AFFECTED(ch, AFF_SNEAK))
    {
	stc("You stop sneaking.\n\r", ch);
	affect_strip(ch, gsn_sneak);
	REMOVE_BIT(ch->affected_by, AFF_SNEAK);
    }

    if (victim->fighting == NULL)
        set_fighting(victim, ch);
    if (ch->fighting == NULL)
       set_fighting(ch, victim);
		
	ch->position = POS_FIGHTING;
	victim->position = POS_FIGHTING;

    if (ch->desc == NULL)
	ch->wait = UMAX(0, ch->wait - PULSE_PER_SECOND);

    if (ch->desc == NULL)
	ch->daze = UMAX(0, ch->daze - PULSE_PER_SECOND);

    if (IS_SPEC(ch, SPEC_PETRIFICATION) && dice(1, 10) == 1
	&& ch->fighting != NULL)
    {
	petrify(ch, victim);
	return;
    }

    crash("char attack 4");

    if (ch->spikes > 0 && dice(1, 2) == 1)
    {
	send_to_char("You pull one of the spikes out of your chest.\n\r",ch);
	act("$n pulls a spike from $s chest and throws it to the floor.",ch, NULL, NULL, TO_ROOM);
	ch->spikes -= 1;
	return;
    }

    if (IS_NPC(ch) && (ch->level > 100 || IS_SET(ch->off_flags, OFF_BERSERK)) && dice(1, 9) == 3 )
    {
        /* rem by whois */
	/* do_berserk(ch, "auto"); */
	return;
    }

    crash("char attack 5");

    if (ch->power[DISC_DAEM_HELL] >= 4 && dice(1, 30-ch->generation) == 1
	&& ch->fighting != NULL)
    {
	int dam;
	char buf[200];

	dam = dice(ch->generation + 1, 20);

	act("#1$n breathes out a cloud of fire at you.",ch,NULL,victim,TO_VICT);
	act("#1$n breathes out a cloud of fire at $N.",ch,NULL,victim,TO_NOTVICT);
	act("#1You breathe out a cloud of fire at $N.",ch,NULL,victim,TO_CHAR);
	sprintf(buf, "The #1flames#n surround $N, %s $M.", damtext_vp(dam) );
	act(buf, ch, NULL, victim, TO_CHAR);
	sprintf(buf, "The #1flames#n surround you, %s you.\n\r", damtext_vp(dam) );
	stc(buf, victim);
	deal_damage(ch, victim, dam );
	if (victim == NULL || victim->in_room == NULL) return;
    }

    if (IS_SPEC(ch, SPEC_FIRE_SPIKES) && dice(1, 20) == 1
	&& ch->fighting != NULL)
    {
	act("$n fires a spike from $s tail at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n fires a spike from $s tail at you.",ch,NULL,victim,TO_VICT);
	act("You fire a spike at $N.",ch,NULL,victim,TO_CHAR);
	victim->spikes += 1;
	damage_old(ch, victim, dice(6, 10), 0, DAM_PIERCE, FALSE);
	if (victim == NULL || victim->in_room == NULL) return;
    }

    /* check for movement */

    crash("char attack 6");
    rate = advance_rate( ch, victim );
    if (IS_FLAG5(victim, AFF5_AIR_BARRIER) && ch->distance <= 4 && rate < 0)
    {
	rate = 0;
	stc("You cannot get past the barrier of air.\n\r",ch);
	act("$n tries to get past the barrier of air, but is thrown back.", ch, NULL, NULL, TO_ROOM);
    }

    ch->distance += rate;
    if (ch->distance < 0) { ch->distance = 0; rate = 0; }
    victim->distance = ch->distance;
    if (rate < 0)
    {
	int mesg = dice(1, 3);
	if (mesg == 1)
	{
	    act("You advance on $N.", ch, NULL, victim, TO_CHAR); 
	    act("$n advances on you.", ch, NULL, victim, TO_VICT); 
	    act("$n advances on $N.", ch, NULL, victim, TO_NOTVICT);
	}
	else if (mesg == 2)
	{
	    act("You circle round $N, trying to get in a better position.", ch, NULL, victim, TO_CHAR);
	    act("$n circles round you, trying to get in a better position.", ch, NULL, victim, TO_VICT);
	    act("$n circles round $N, trying to get in a better position.", ch, NULL, victim, TO_NOTVICT);
	}
	else
	{
	    act("You taunt $N, as you move in closer.", ch, NULL, victim, TO_CHAR);
	    act("$n taunts you, as $e moves in closer.", ch, NULL, victim, TO_VICT);
	    act("$n taunts $N, as $e moves in closer.", ch, NULL, victim, TO_NOTVICT);
	}
    }
    else if (rate > 0)
    {
	act("You back away from $N.", ch, NULL, victim, TO_CHAR); 
	act("$n backs away from you.", ch, NULL, victim, TO_VICT); 
	act("$n backs away from $N.", ch, NULL, victim, TO_NOTVICT); 
    }
    crash("char attack 7");

    /* if crippled, skip combat round */

    if (misc_body_combat_penalty(ch, 100) < 1)
	return;

    // right arm attacks

    if (!BODY_PART(ch, BODY_RIGHT_ARM) && !BODY_PART(ch,BODY_RIGHT_HAND))
    {
	ch->initiative[0] += initiative_gain;
	if (ch->initiative[0] > max_initiative) ch->initiative[0] = max_initiative;

	chance = misc_body_combat_penalty( ch, 66 );
	hand = get_eq_char( ch, WEAR_RIGHT_HAND );
	while (will_attack( ch, ch->initiative[0], initiative_cost(ch, hand)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, hand))
	{
	    /* do attack */
	    ch->initiative[0] -= initiative_cost( ch, hand );

	    perform_hit(ch, victim, hand, TRUE);

	    if (ch->fighting != victim)
		return;
	}
    }

    crash("char attack 8");
    // left arm attacks

    if (!BODY_PART(ch, BODY_LEFT_ARM) && !BODY_PART(ch,BODY_LEFT_HAND))
    {
	ch->initiative[1] += initiative_gain;
	if (ch->initiative[1] > max_initiative) ch->initiative[1] = max_initiative;

	chance = misc_body_combat_penalty( ch, 66 );
	hand = get_eq_char( ch, WEAR_LEFT_HAND );
	while (will_attack( ch, ch->initiative[1], initiative_cost(ch, hand)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, hand))
	{
	    /* do attack */
	    ch->initiative[1] -= initiative_cost( ch, hand );

	    perform_hit(ch, victim, hand, TRUE);

	    if (ch->fighting != victim)
		return;
	}
    }
    crash("char attack 9");

    if (BODY_PART(ch, BODY_THIRD_ARM) && BODY_PART(ch,BODY_THIRD_HAND))
    {
	ch->initiative[2] += initiative_gain * 3 / 4;
	if (ch->initiative[2] > max_initiative) ch->initiative[2] = max_initiative;

	chance = misc_body_combat_penalty( ch, 66 );
	hand = get_eq_char( ch, WEAR_THIRD_HAND );
	while (will_attack( ch, ch->initiative[2], initiative_cost(ch, hand)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, hand))
	{
	    /* do attack */
	    ch->initiative[2] -= initiative_cost( ch, hand );

	    perform_hit(ch, victim, hand, TRUE);

	    if (ch->fighting != victim)
		return;
	}
    }
    crash("char attack 10");

    if (BODY_PART(ch, BODY_FOURTH_ARM) && BODY_PART(ch,BODY_FOURTH_HAND))
    {
	ch->initiative[3] += initiative_gain * 3 / 4;
	if (ch->initiative[3] > max_initiative) ch->initiative[3] = max_initiative;

	chance = misc_body_combat_penalty( ch, 66 );
	hand = get_eq_char( ch, WEAR_FOURTH_HAND );
	while (will_attack( ch, ch->initiative[3], initiative_cost(ch, hand)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, hand))
	{
	    /* do attack */
	    ch->initiative[3] -= initiative_cost( ch, hand );

	    perform_hit(ch, victim, hand, TRUE);

	    if (ch->fighting != victim)
		return;
	}
    }

    // miscellanious body attacks

    ch->initiative[4] += initiative_gain;
    if (ch->initiative[4] > max_initiative) ch->initiative[4] = max_initiative;

    if (dice(1, 20) && ch->distance > ch->des_dist)
    {
	if (will_attack(ch, ch->initiative[4], 15))
	{
	    ch->initiative[0] = 0;
	    ch->initiative[1] = 0;
	    ch->initiative[2] = 0;
	    ch->initiative[3] = 0;
	    ch->initiative[4] = 0;
	    ch->distance = ch->des_dist;
	    victim->distance = ch->distance;
	    act("$n charges $N dodging their weapon.", ch, NULL, victim, TO_NOTVICT);
	    act("You charge at $N dodging their weapon.", ch, NULL, victim, TO_CHAR);
	    act("$n charges at you dodging your weapon.", ch, NULL, victim, TO_VICT);
	}
    }

    crash("char attack 14");

    // bash
    crash("char attack 16");

    if (dice(1, 16) == 3 && IS_NPC(ch))
    {
	do_kick(ch, "");
    }

    if (dice(1, 16) == 3)
    {
	chance = misc_body_combat_penalty( ch, 66 );
	if (will_attack( ch, ch->initiative[4], initiative_cost(ch, NULL)) && dice(1, 100) < chance && (ch->distance <= get_size(ch) || ch->distance <= get_size(victim)))
	{
		dam = 0;
		sn = gsn_bash;
		dt = TYPE_HIT + ch->dam_type;
		dam_type = DAM_BASH;
		ch->initiative[4] -= initiative_cost(ch, NULL);
		dam = get_punch_dam(ch);
		if (ch->cur_form == FRM_CRIMSON)
		{
		    dam_type = DAM_NEGATIVE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_WHIRLWIND)
		{
		    dam_type = DAM_BASH;
		    dam = dam * 6 / 5;
		}
		else if (ch->cur_form == FRM_FLAME)
		{
		    dam_type = DAM_FIRE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_LIGHT)
		{
		    dam_type = DAM_LIGHTNING;
		    dam = dam * 5 / 4;
		}

		if (get_skill(ch, gsn_enhanced_damage) > 0)
		{
		    diceroll = number_percent();
		    if (diceroll <= get_skill(ch, gsn_enhanced_damage))
		    {
			check_improve(ch, gsn_enhanced_damage,TRUE,6);
			dam += 2 * (dam * diceroll / 300);
		    }
		}

		    if (stances[ch->stance].score[victim->stance] >= 0)
			stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
		    else
		    {
			stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
			if (stmod < 10) stmod = 10;
		    }
		    if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
		    {
			if (dice(1, 2) == 1)
			    improve_stance(ch, ch->stance);
		    }
		    dam = (dam * stmod) / 100;

		dam += GET_DAMROLL(ch) / 5;

		if ((didhit=did_hit(ch, victim, sn, dam_type)) > 0)
		{
		    if (didhit == 19) dam *= 2;
		    dam = cap_dam(ch, victim, dam, dam_type, TRUE);
	 	    crash("called cap dam place 6");
		    attack_modify = dice(1, 100);
		    dam = randomize_damage( ch, dam, attack_modify );
		    if (attack_modify > 25)
		    {
			ch->distance += 1;
			victim->distance += 1;
		    } else if (attack_modify > 75)
		    {
			ch->distance += 2;
			victim->distance += 2;
		    }
		    display_bash_damage(ch, victim, dam);
		    stunning_hit(ch, victim);
		    if (deal_damage(ch, victim, dam)) return;
			    if (dam > 200 && victim->distance < MAX_DIST)
			    {
				ch->distance += dam / 100;
				if (ch->distance > MAX_DIST) ch->distance = MAX_DIST;
				victim->distance = ch->distance;
				stc("You are thrown backwards by the force of the blow.\n\r", victim);
				act("$n is thrown back by the force of the blow.", victim, NULL, NULL, TO_ROOM);
			    }

		}
		else
		{
		    display_bash_miss(ch, victim);
		}
	    }
	    if (ch->fighting != victim)
		return;
    }

    if (dice(1, 4) == 3 && IS_SET(ch->flag2, AFF_BITE))
    {
	chance = misc_body_combat_penalty( ch, 66 );
	if (will_attack( ch, ch->initiative[4], initiative_cost(ch, NULL)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, NULL))
	{
		dam = 0;
		sn = gsn_bash;
		dt = TYPE_HIT + ch->dam_type;
		dam_type = DAM_PIERCE;
		ch->initiative[4] -= initiative_cost(ch, NULL);
		dam = get_punch_dam(ch);
		if (ch->cur_form == FRM_CRIMSON)
		{
		    dam_type = DAM_NEGATIVE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_WHIRLWIND)
		{
		    dam_type = DAM_BASH;
		    dam = dam * 6 / 5;
		}
		else if (ch->cur_form == FRM_FLAME)
		{
		    dam_type = DAM_FIRE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_LIGHT)
		{
		    dam_type = DAM_LIGHTNING;
		    dam = dam * 5 / 4;
		}

		if (get_skill(ch, gsn_enhanced_damage) > 0)
		{
		    diceroll = number_percent();
		    if (diceroll <= get_skill(ch, gsn_enhanced_damage))
		    {
			check_improve(ch, gsn_enhanced_damage,TRUE,6);
			dam += 2 * (dam * diceroll / 300);
		    }
		}

		    if (stances[ch->stance].score[victim->stance] >= 0)
			stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
		    else
		    {
			stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
			if (stmod < 10) stmod = 10;
		    }
		    if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
		    {
			if (dice(1, 2) == 1)
			    improve_stance(ch, ch->stance);
		    }
		    dam = (dam * stmod) / 100;

		dam += GET_DAMROLL(ch) / 5;

		dam = dam * get_size(ch) / 3;

		if ((didhit=did_hit(ch, victim, sn, dam_type)) > 0)
		{
		    if (didhit == 19) dam *= 2;
		    dam = cap_dam(ch, victim, dam, dam_type, TRUE);
	 	    crash("called cap dam place 6");
		    attack_modify = dice(1, 100);
		    dam = randomize_damage( ch, dam, attack_modify );
		    if (attack_modify > 25)
		    {
			ch->distance += 1;
			victim->distance += 1;
		    } else if (attack_modify > 75)
		    {
			ch->distance += 2;
			victim->distance += 2;
		    }
		    display_bite_damage(ch, victim, dam);
		    if (deal_damage(ch, victim, dam)) return;
		}
		else
		{
		    display_bite_miss(ch, victim);
		}
	    }
	    if (ch->fighting != victim)
		return;
    }

    if (dice(1, 4) == 3 && IS_SET(ch->flag2, AFF_TAIL))
    {
	chance = misc_body_combat_penalty( ch, 66 );
	if (will_attack( ch, ch->initiative[4], initiative_cost(ch, NULL)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, NULL))
	{
		dam = 0;
		sn = gsn_bash;
		dt = TYPE_HIT + ch->dam_type;
		dam_type = DAM_PIERCE;
		ch->initiative[4] -= initiative_cost(ch, NULL);
		dam = get_punch_dam(ch);
		if (ch->cur_form == FRM_CRIMSON)
		{
		    dam_type = DAM_NEGATIVE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_WHIRLWIND)
		{
		    dam_type = DAM_BASH;
		    dam = dam * 6 / 5;
		}
		else if (ch->cur_form == FRM_FLAME)
		{
		    dam_type = DAM_FIRE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_LIGHT)
		{
		    dam_type = DAM_LIGHTNING;
		    dam = dam * 5 / 4;
		}

		if (get_skill(ch, gsn_enhanced_damage) > 0)
		{
		    diceroll = number_percent();
		    if (diceroll <= get_skill(ch, gsn_enhanced_damage))
		    {
			check_improve(ch, gsn_enhanced_damage,TRUE,6);
			dam += 2 * (dam * diceroll / 300);
		    }
		}

		    if (stances[ch->stance].score[victim->stance] >= 0)
			stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
		    else
		    {
			stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
			if (stmod < 10) stmod = 10;
		    }
		    if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
		    {
			if (dice(1, 2) == 1)
			    improve_stance(ch, ch->stance);
		    }
		    dam = (dam * stmod) / 100;

		dam += GET_DAMROLL(ch) / 5;

		dam = dam * get_size(ch) / 3;

		if ((didhit=did_hit(ch, victim, sn, dam_type)) > 0)
		{
		    if (didhit == 19) dam *= 2;
		    dam = cap_dam(ch, victim, dam, dam_type, TRUE);
	 	    crash("called cap dam place 6");
		    attack_modify = dice(1, 100);
		    dam = randomize_damage( ch, dam, attack_modify );
		    if (attack_modify > 25)
		    {
			ch->distance += 1;
			victim->distance += 1;
		    } else if (attack_modify > 75)
		    {
			ch->distance += 2;
			victim->distance += 2;
		    }
		    display_tail_damage(ch, victim, dam);
		    if (deal_damage(ch, victim, dam)) return;
		}
		else
		{
		    display_tail_miss(ch, victim);
		}
	    }
	    if (ch->fighting != victim)
		return;
    }

    if (dice(1, 9) == 3 && IS_SET(ch->flag2, AFF_WING))
    {
	chance = misc_body_combat_penalty( ch, 66 );
	if (will_attack( ch, ch->initiative[4], initiative_cost(ch, NULL)) && dice(1, 100) < chance && ch->distance <= attack_reach(ch, NULL))
	{
		dam = 0;
		sn = gsn_bash;
		dt = TYPE_HIT + ch->dam_type;
		dam_type = DAM_PIERCE;
		ch->initiative[4] -= initiative_cost(ch, NULL);
		dam = get_punch_dam(ch) * (get_size(ch) - get_size(victim));
		if (ch->cur_form == FRM_CRIMSON)
		{
		    dam_type = DAM_NEGATIVE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_WHIRLWIND)
		{
		    dam_type = DAM_BASH;
		    dam = dam * 6 / 5;
		}
		else if (ch->cur_form == FRM_FLAME)
		{
		    dam_type = DAM_FIRE;
		    dam = dam * 3 / 2;
		}
		else if (ch->cur_form == FRM_LIGHT)
		{
		    dam_type = DAM_LIGHTNING;
		    dam = dam * 5 / 4;
		}

		if (get_skill(ch, gsn_enhanced_damage) > 0)
		{
		    diceroll = number_percent();
		    if (diceroll <= get_skill(ch, gsn_enhanced_damage))
		    {
			check_improve(ch, gsn_enhanced_damage,TRUE,6);
			dam += 2 * (dam * diceroll / 300);
		    }
		}

		    if (stances[ch->stance].score[victim->stance] >= 0)
			stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
		    else
		    {
			stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
			if (stmod < 10) stmod = 10;
		    }
		    if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
		    {
			if (dice(1, 2) == 1)
			    improve_stance(ch, ch->stance);
		    }
		    dam = (dam * stmod) / 100;

		dam += GET_DAMROLL(ch) / 5;

		if ((didhit=did_hit(ch, victim, sn, dam_type)) > 0)
		{
		    if (didhit == 19) dam *= 2;
		    dam = cap_dam(ch, victim, dam, dam_type, TRUE);
	 	    crash("called cap dam place 6");
		    attack_modify = dice(1, 100);
		    dam = randomize_damage( ch, dam, attack_modify );
		    if (attack_modify > 25)
		    {
			ch->distance += 1;
			victim->distance += 1;
		    } else if (attack_modify > 75)
		    {
			ch->distance += 2;
			victim->distance += 2;
		    }
		    display_wing_damage(ch, victim, dam);
		    if (deal_damage(ch, victim, dam)) return;
		    stunning_hit(ch, victim);
		}
		else
		{
		    display_wing_miss(ch, victim);
		}
	    }
	    if (ch->fighting != victim)
		return;
    }


/* pulled by whois -- berserk is evil
    if (victim->hit < victim->max_hit < 5 && IS_NPC(victim) && IS_FLAG6(ch, AFF6_AUTO_ATTACK) && dice(1, 10) < 4)
    {
	do_berserk(victim, "auto mob");
    }
*/
   
    crash("char attack 17");
    
    if (IS_FLAG4(ch, AFF_STASIS))
    {
    	stop_fighting(ch, TRUE);
	return;
    }

// end of combat routine - find by searching for bugaboo
}

int randomize_damage( CHAR_DATA *ch, int dam, int am )
{
    dam = dam * (am + 50) / 100;

// damage weapon, limb loss etc.
    return dam;
}

int did_hit(CHAR_DATA *ch, CHAR_DATA *victim, int sn, int dam_type)
{
    int thac0, vict_ac = 0, diceroll;

    crash("did hit?");

    thac0 = calc_thaco( ch ) - GET_HITROLL(ch) - get_weapon_skill(ch, sn);
    thac0 -= stances[ch->stance].score[victim->stance] / 2;

    switch(dam_type)
    {
	case (DAM_PIERCE):vict_ac = GET_AC(victim, AC_PIERCE)/10;break;
	case (DAM_BASH)  :vict_ac = GET_AC(victim, AC_BASH)  /10;break;
	case (DAM_SLASH) :vict_ac = GET_AC(victim, AC_SLASH) /10;break;
	default		 :vict_ac = GET_AC(victim, AC_EXOTIC)/10;break;
    }

    if (vict_ac < -15)
	vict_ac = (vict_ac + 15) / 3 - 15;

    if (!can_see(ch, victim) )
    {
	vict_ac -= 100 - get_skill(ch, gsn_blindfighting);
	check_improve(ch, gsn_blindfighting, FALSE, 3);
    }

    if (!IS_NPC(victim) && victim->pcdata->condition[COND_DRUNK] > 10)
	vict_ac += 50;

    if (victim->hit < 1) vict_ac += 100;

    crash("did hit? 2");

    if (victim->power[DISC_ELAD_PHYS] > 4)
	vict_ac -= victim->played / 360000;

    if (victim->position < POS_FIGHTING)
	vict_ac += 4;

    if (victim->position < POS_RESTING)
	vict_ac += 100;

    if (!RTIME_UP(victim->in_room, RTIMER_GLYPH_PROTECTION)
	&& IS_GOOD(victim))
	vict_ac -= 40;

    crash("did hit? 2 and a half");

    while( ( diceroll = number_bits( 50 ) ) >= 20) ;

    if (diceroll == 0
	|| (diceroll != 19 && diceroll < thac0 - vict_ac))
	return 0;

    crash("did hit? 3");

    if (diceroll == 19 && diceroll >= thac0 - vict_ac)
	return 2;
    else if (diceroll == 0)
	return -1;

    return 1;
}

int cap_dam(CHAR_DATA *ch, CHAR_DATA *vict, int dam, int dam_type, bool fitness)
{
    int sizedif;

    crash("cap dam");

    if (is_wearing_artifact(vict, ARTIFACT_JANDOR) != NULL)
	return 0;

    if (IS_FLAG4(vict, AFF_INVULNERABLE))
	return 0;

    if (is_safe(ch, vict))
	return 0;

    if (vict->stance == 6)
    	dam = dam * (100 - ch->stance_skill[6] / 2) / 100;
    if (ch->stance == 10)
	dam = dam * (100 + ch->stance_skill[10] / 2) / 100;

    // reduce damage to %age vitality / 2 + 50
    // increase damage take by (100 - %age vitality / 2) + 100%

    if (fitness)
    {
	dam = dam * ((( ch->move * 100 / ch->max_move)) / 2 + 50) / 100;
	dam = dam * (( 100 - ( vict->move * 100 / vict->max_move)) / 2 + 100) / 100;
    }

    switch(dam_type)
    {
	case DAM_PIERCE :
	    dam += (ch->style_skill[2] * 3) - (vict->style_skill[3] * 3);
	    break;
	case DAM_BASH :
	    dam += (ch->style_skill[5] * 3) - (vict->style_skill[4] * 3);
	    break;
	case DAM_SLASH :
	    dam += (ch->style_skill[1] * 3) - (vict->style_skill[0] * 3);
	    break;
	default:
	    dam += (ch->style_skill[6] * 3) - (vict->style_skill[7] * 3);
	    break;
    }

    crash("cap dam place 2");
    // lets get those auras working now!

    if (vict->secondaryrace == RACE_ELADRIN && vict->aura > 0)
    {
	if (vict->aura >= 10) dam /= 3;
	else if (vict->aura > 6) dam /= 2;
	else if (vict->aura > 2) dam = (dam * 2) / 3;
    }
    crash("cap dam place 2 and a half");

    /* The higher the number, the more damage done to the victim */
  
    if (dam_type == DAM_SLASH || dam_type == DAM_BASH || dam_type == DAM_PIERCE)
    {
	/* Imunae does physical attacks only */
	if (vict->power[DISC_DAEM_IMUN] > 0)
  	{
	    dam *= 20 - ch->power[DISC_DAEM_IMUN];
	    dam /= 20;
 	}
        crash("cap dam place 2 and a half and a bit extra");

        sizedif = get_size(ch) - get_size(vict);
        if (sizedif == 5) dam *= 5;
        else if (sizedif == 4) dam *= 4;
        else if (sizedif == 3) dam *= 2;
        else if (sizedif == 2) dam = (dam * 3) / 2;
        else if (sizedif == 1) dam = (dam * 5) / 4;
        else if (sizedif == -1) dam = (dam * 4) / 5;
        else if (sizedif == -2) dam = (dam * 2) / 3;
        else if (sizedif == -3) dam /= 2;
        else if (sizedif == -4) dam /= 4;
        else if (sizedif == -5) dam /= 5;
    }

    crash("cap dam place 3");

    if (dam > 80)
	dam = (dam - 80) / 2 + 80;

    if (ch->secondaryrace == RACE_LUPINE
	&& (time_info.day > 33 || time_info.day < 2) )
	dam *= 2;
    if (ch->secondaryrace == RACE_LUPINE
	&& (time_info.day == 18 || time_info.day == 17) )
	dam /= 2;

    if (!IS_NPC(vict) && vict->pcdata->condition[COND_DRUNK] > 10)
	dam /= 2;

    crash("cap dam place 4");
    if (IS_AFFECTED(vict, AFF_SANCTUARY))
	dam /= 2;

    /* Vampire protean 7 (flesh of lava) reduces damage */
    if (vict->power[DISC_VAMP_PROT] >= 7)
    {
  	dam *= 2;
	dam /= 3;
    }

    /* Vampire fortitude reduces damage
	and so does the FaKiR for Warlocks!  But not both 
	and so does boar - to a lesser extent*/

    if (ch->power[DISC_VAMP_FORT] > 0)
    {
        dam *= (100 - (ch->power[DISC_VAMP_FORT] * 6));
        dam /= 100;
    }
    else if (ch->power[DISC_WARL_FAKI] > 0)
    {
        dam *= (100 - (ch->power[DISC_WARL_FAKI] * 6));
        dam /= 100;
    }
    else if (ch->power[DISC_LUPI_BOAR] > 0)
    {
        dam *= (100 - (ch->power[DISC_LUPI_BOAR] * 6));
        dam /= 100;
    }

    /* Nasty form spells to avoid damage */
    if (vict->cur_form == FRM_TREE)
  	dam /= 3;
    if (vict->cur_form == FRM_GOLEM)
  	dam /= 3;

    crash("cap dam place 5");
    if ( dam > 1 && ((IS_AFFECTED(vict, AFF_PROTECT_EVIL) && IS_EVIL(ch))    
    ||               (IS_AFFECTED(vict, AFF_PROTECT_GOOD) && IS_GOOD(ch))))  
        dam -= dam / 4;      

    if (dam <= 0)
	dam = 1;

    switch(check_immune(vict, dam_type))
    {
	case (IS_IMMUNE)    : return 0;       break;
	case (IS_RESISTANT) : dam -= dam / 2; break;
	case (IS_VULNERABLE): dam += dam / 2; break;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ETHERIAL))
	return 0;

    if (IS_FLAG4(vict, AFF_STASIS))
    {
	stop_fighting(vict, TRUE);
	return 0;
    }

//    if (!IS_NPC(ch) && ch->level != 120 && dam > ch->pcdata->damcap)        
//        dam = ch->pcdata->damcap;                                           
                                                                               
    if (dam > 30000) dam = 30000;                                               

    crash("cap dam place 6");
    return dam;
}

bool deal_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    OBJ_DATA *corpse;

    crash("deal damage");

    /* You cant hurt objects... yeah man go go underground */
    if (is_item(victim)) return FALSE;

    if (!IS_NPC(ch) && !IS_NPC(victim))
	dam = dam / 8;

    if (dam < 1) dam = 1;

    victim->hit -= dam;

    if (victim->hit < 1 && !TIME_UP(victim, TIMER_NEWBIE_IMM) )
    {
	stc("As you are new to this world, the gods give you one more chance.\n\r", victim);
	stc("Bright lights surround you, sparkling and humming gently.\n\r", victim);
	stc("You have been fully restored.\n\r", victim);
	act("The gods smile upon $n, as they are new to this world.", victim, NULL, NULL, TO_ROOM);
	act("$n has been restored to full health.", victim, NULL, NULL, TO_ROOM);
	restore_char(victim);
	update_pos(victim);
	stop_fighting(victim, TRUE);
	return FALSE;
    }

    /* Retribution... ok */

    if (IS_FLAG4(victim, AFF_RETRIBUTION)
        && ch->secondaryrace != 0)
    {
        REMOVE_BIT(victim->flag4, AFF_RETRIBUTION);
        affect_strip(victim, gsn_retribution);
        act("$n is struck down by a lightning bolt from the heavens.", ch, NULL, NULL, TO_ROOM);
        send_to_char("You are struck down by a lightning bolt from the heavens.\n\r", ch);
        damage_old(victim, ch, dam * 10, skill_lookup("retribution"), DAM_LIGHTNING, TRUE);
        stop_fighting(ch, TRUE);
        return TRUE; 
    }
    crash("deal damage 2");

    if (!IS_NPC(victim)
	&& victim->level >= LEVEL_IMMORTAL
	&& victim->hit < 1)
	    victim->hit = 1;

    /* check for familiars etc... */
    if (victim->hit < 1
	&& victim->pet != NULL
	&& IS_FLAG5(victim->pet, AFF5_FAMILIAR))
	    victim->hit = 1;

    update_pos( victim );
    crash("deal damage 3");

    switch( victim->position )                                                  
    {                                                                           
    case POS_MORTAL:                                                            
        act( "#1$n is mortally wounded, and will die soon, if not aided.",      
            victim, NULL, NULL, TO_ROOM );                                      
        send_to_char(                                                           
            "#1You are mortally wounded, and will die soon, if not aided.\n\r", 
            victim );                                                           
        break;                                                                  
                                                                                
    case POS_INCAP:                                                             
        act( "#1$n is incapacitated and will slowly die, if not aided.",        
            victim, NULL, NULL, TO_ROOM );                                      
        send_to_char(                                                           
            "#1You are incapacitated and will slowly die, if not aided.\n\r",   
            victim );                                                           
        break;                                                                  
                                                                                
    case POS_STUNNED:                                                           
        act( "#3$n is stunned, but will probably recover.",                     
            victim, NULL, NULL, TO_ROOM );                                      
        send_to_char("#3You are stunned, but will probably recover.\n\r",      
            victim );                                                           
        break;                                                                  
                                                                                
    case POS_DEAD:                                                              
        death_move(ch, victim);                                                 
        death_cry(victim, TRUE);                                                
        act( "#1$n drops to the floor DEAD!!", victim, 0, 0, TO_ROOM );         
        send_to_char( "#1You have been KILLED!!\n\r\n\r", victim );             
        stop_fighting(victim, TRUE);                                            
        break;                                                                  
                                                                               
    default:                                                                    
        if ( dam > victim->max_hit / 4 )                                        
            send_to_char( "That really did HURT!\n\r", victim );                
        if ( victim->hit < victim->max_hit / 4 && dam > 10) 
            send_to_char( "#1You sure are BLEEDING!\n\r", victim );             
        break;                                                                  
    }                                                                           
                                                                               
    if (victim->position == POS_DEAD && !IS_NPC(victim) &&                      
        (victim->secondaryrace!=0 || victim->hit + victim->illus_dam > -10))    
    {                                                                           
        stop_fighting(victim, TRUE);                                            
        victim->hit = -10 - victim->illus_dam;                                  
        if (ch->secondaryrace != RACE_WARLOCK) wipe_spells(victim);             
        return TRUE; 
    }                                                                           
    else                                                                        
    {                                                                           
        update_pos(victim);                                                     
    }                                                                           
    crash("deal damage 4");
                                                                                
    if ( !IS_AWAKE(victim) )                                                    
        stop_fighting( victim, FALSE );                                         
                                                                                
    if (IS_FLAG4(ch, AFF_STASIS)) stop_fighting(victim, FALSE);                 
    if (IS_FLAG4(victim, AFF_STASIS)) stop_fighting(victim, FALSE);             
 

    /*                                                                          
     * Payoff for killing things.                                               
     */                                                                         
    crash("deal damage 5");
                                                                                
    if ( victim->position == POS_DEAD && victim->secondaryrace==0)              
    {                                                                           
        check_quest_kill(ch, victim);                                           

	crash("kippers?  not on a tuesday");
                                                                                
        if (IS_SPEC(victim, SPEC_EVERLIVING))                                   
        {                                                                       
            victim->hit = victim->max_hit;                                      
            victim->move = victim->max_move;                                    
            stop_fighting(victim, TRUE);                                        
            update_pos(victim);                                                 
            act("#2$n gets back to $s feet, and all $s wounds heal up.",victim, NULL, NULL, TO_ROOM);
            return TRUE;                                                        
        }                                                                       
                                                                                
        if (IS_SPEC(victim, SPEC_SPLIT))                                        
        {                                                                     
            CHAR_DATA *mob = NULL;                                              
                                                                                
            victim->max_hit /= 2;                                               
            if (victim->max_hit > 30)                                           
            {                                                                   
                clone_mobile( victim, mob );                                    
                if (mob == NULL) return TRUE;                                   
                mob->max_hit = victim->max_hit;                                 
                victim->hit = victim->max_hit;                                  
                mob->hit = mob->max_hit;                                        
                victim->move = victim->max_move;                                
                mob->move = mob->max_move;                                      
                char_to_room(mob, ch->in_room, ch->altitude);                   
                set_fighting(mob, ch);                                          
                set_fighting(victim, ch);                                       
                update_pos(victim);                                             
                update_pos(mob);                                                
                act("#2$n splits in half and both pieces continue fighting.", victim, NULL, NULL, TO_ROOM);
                return FALSE;                                                    
             }                                                              
        }                                                                       
                                                                                
	crash("jam and ham and spam");
        group_gain( ch, victim );                                               
                                                                                
        if (!IS_NPC(ch) && victim->level > ch->best_mob_killed)                 
        {                                                                       
            ch->best_mob_killed = victim->level;                                
            check_player_table(ch);                                             
        }                                                                       

	/* Help the eladrins autopromote */
	if (ch->secondaryrace == RACE_ELADRIN
	    && ch->tertiaryrace == 0
	    && ch->in_room->sector_type == SECT_FOREST
	    && IS_EVIL(victim)
	    && victim->level > ch->level / 2)
	    gain_elad_path(ch, 1);

	crash("yes sir, put your feet into that please");
        if (IS_NPC(victim) && !IS_NPC(ch) )                                     
            ch->pcdata->mob_kills += 1;                                         
                                                                                
        if (IS_SPEC(victim, SPEC_FREEZE)) WAIT_STATE(ch, 18);                   
                                                                                
	crash("the cat in the hat");

        if ( !IS_NPC(victim) )                                                  
        {                                                                       
            if (IS_NPC(ch))                                                     
                victim->pcdata->mob_killed += 1;                                
            sprintf( log_buf, "%s killed by %s at %d",                          
                victim->name,                                                   
                (IS_NPC(ch) ? ch->short_descr : ch->name),                      
                ch->in_room->vnum );                                            
            log_string( log_buf );                                              
        }                                                                       
                                                                                
        sprintf( log_buf, "%s got toasted by %s at %s [room %d]",               
            (IS_NPC(victim) ? victim->short_descr : victim->name),              
            (IS_NPC(ch) ? ch->short_descr : ch->name),                          
            ch->in_room->name, ch->in_room->vnum);                              
                                                                                
        if (IS_NPC(victim))                                                     
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);                        
        else                                                                    
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);                           
                                                                                
	crash("chris smells of dung beetles");

        raw_kill( victim );                                                     
        if (!IS_NPC(ch)                                                         
        &&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL  
        &&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))     
        {                                                                       
            OBJ_DATA *coins;                                                    
                                                                                
            corpse = get_obj_list( ch, "corpse", ch->in_room->contents );       
                                                                                
            if ( IS_SET(ch->act, PLR_AUTOLOOT) &&                               
                 corpse && corpse->contains) /* exists and not empty */         
                do_get( ch, "all corpse" );                                     
                                                                                
            if (IS_SET(ch->act,PLR_AUTOGOLD) &&                                 
                corpse && corpse->contains  && /* exists and not empty */       
                !IS_SET(ch->act,PLR_AUTOLOOT))                                  
                if ((coins = get_obj_list(ch,"gcash",corpse->contains))         
                     != NULL)                                                   
                    do_get(ch, "all.gcash corpse");                             
                                                                                
            if ( IS_SET(ch->act, PLR_AUTOSAC) )                                 
              if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)  
                return TRUE;  /* leave if corpse has treasure */                
              else                                                            
                do_sacrifice( ch, "corpse" );                                   
        }                                                                       
        return TRUE;                                                            
    }                                                                           

    crash("like silly putty?  i love it!");

    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)        
    {                                                                           
        if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0          
        &&   victim->hit < victim->max_hit / 5)                                 
        ||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL         
        &&     victim->master->in_room != victim->in_room ) )                   
            do_flee( victim, "" );                                              
    }                                                                           
                                                                                
    if ( !IS_NPC(victim)                                                        
    &&   victim->hit > 0                                                        
    &&   victim->hit <= victim->wimpy                                           
    &&   victim->wait < PULSE_VIOLENCE / 2 )                                    
        do_flee( victim, "" );                                                  

    crash("no, the margarine goes on top of the fridge!");
                                                                                
    tail_chain( );                                                              
    return FALSE;                                                  
}                                                                               

void display_weapon_damage(CHAR_DATA *ch, CHAR_DATA *victim, 
		OBJ_DATA *wield, int dam, int atm)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    const char *vs, *attack, *vp, *v, *attacks;
    char *weap;
    int dt, mesg;

    crash("Display weapon damage");

    if (ch == NULL || victim == NULL || wield == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    weap = wield->short_descr;
    if (weap[0] == 'a' && weap[1] == ' ')
	weap = &weap[2];

    dt = TYPE_HIT + wield->value[3];
    if (dt < TYPE_HIT)
    {
    	attack = attack_table[wield->value[3]].noun;
    	attacks = attack_table[wield->value[3]].noun_p;
    }
    else
    {
	attack = attack_table[dt - TYPE_HIT].noun;
	attacks = attack_table[dt - TYPE_HIT].noun_p;
    }

    crash("Display weapon damage2");

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch == victim)
	{
	    sprintf(buf1, "$n's %s passes harmlessly through $mself.",weap);
	    sprintf(buf2, "Luckily, your %s passes harmlessly through you.",weap);
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	}
	else
	{
	    sprintf(buf1,"#2$n's %s passes harmlessly through you.",weap);
	    sprintf(buf2,"#2Your %s passes harmlessly through $N.",weap);
	    sprintf(buf3,"#2$n's %s passes harlessly through you.",weap);
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    crash("Display weapon damage3");

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own %s.",attack);
	    sprintf(buf2,"#2Luckily, you are immune to taht.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's %s!",attack);
	    sprintf(buf2,"#2$N is unaffected by your %s!",attack);
	    sprintf(buf3,"#2$n's %s is powerless against you.",attack);
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }
    else if (dam <=   4) { vs = "bruises";	vp = "bruising";	
			   v  = "bruise"; }
    else if (dam <=   6) { vs = "scratches";	vp = "scratching";	
			   v  = "scratch"; }
    else if (dam <=   8) { vs = "grazes";	vp = "grazing";		
			   v  = "graze"; }
    else if (dam <=  12) { vs = "hits";		vp = "hitting";		
			   v  = "hit"; }
    else if (dam <=  16) { vs = "injures";	vp = "injuring";	
			   v  = "injure"; }
    else if (dam <=  20) { vs = "wounds";	vp = "wounding";	
			   v  = "wound"; }
    else if (dam <=  24) { vs = "mauls";	vp = "mauling";		
			   v  = "maul"; }
    else if (dam <=  28) { vs = "decimates";	vp = "decimating";	
			   v  = "decimate"; }
    else if (dam <=  32) { vs = "devastates";	vp = "devastating";	
			   v  = "devastate"; }
    else if (dam <=  36) { vs = "maims";	vp = "maiming";	
			   v  = "maim"; }
    else if (dam <=  42) { vs = "MUTILATES";	vp = "MUTILATING";	
			   v  = "MUTILATE"; }
    else if (dam <=  48) { vs = "DISEMBOWELS";	vp = "DISEMBOWELING";	
			   v  = "DISEMBOWEL"; }
    else if (dam <=  54) { vs = "DISMEMBERS";	vp = "DISMEMBERING";	
			   v  = "DISMEMBER"; }
    else if (dam <=  60) { vs = "MASSACRES";	vp = "MASSACRING";	
			   v  = "MASSACRE"; }
    else if (dam <=  68) { vs = "MANGLES";	vp = "MANGLING";	
			   v  = "MANGLE"; }
    else if (dam <=  76) { vs = "> CRUSHES <";	vp = "> CRUSHING <";	
			   v  = "> CRUSH <"; }
    else if (dam <=  86) { vs = "+- DESECRATES -+";
			   vp = "+- DESECRATING -+";	
			   v  = "+- DESECRATE -+"; }
    else if (dam <= 100) { vs = ">- DEMOLISHES -<";
			   vp = ">- DEMOLISHING -<";	
			   v  = ">- DEMOLISH -<"; }
    else if (dam <= 110) { vs = "*** DEVASTATES ***";
			   vp = "*** DEVASTATING ***";	
			   v  = "*** DEVASTATE ***"; }
    else if (dam <= 130) { vs = "*** EVISCERATES ***";
			   vp = "*** EVISCERATING ***";	
			   v  = "*** EVISCERATE ***"; }
    else if (dam <= 160) { vs = "=== OBLITERATES ===";
			   vp = "=== OBLITERATING ===";	
			   v  = "=== OBLITERATE ==="; }
    else if (dam <= 200) { vs = ">>> ANNIHILATES <<<";
			   vp = ">>> ANNIHILATING <<<";	
			   v  = ">>> ANNIHILATE <<<"; }
    else if (dam <= 240) { vs = "<<< ERADICATES >>>";
			   vp = "<<< ERADICATING >>>";	
			   v  = "<<< ERADICATE >>>"; }
    else if (dam <= 280) { vs = ">-> LIQUEFIES <-<";
			   vp = ">-> LIQUEFYING <-<";	
			   v  = ">-> LIQUEFY <-<"; }
    else if (dam <= 320) { vs = "ooo PULVERISES ooo";
			   vp = "ooo PULVERISING ooo";	
			   v  = "ooo PULVERISE ooo"; }
    else if (dam <= 360) { vs = "<*<*< NUKES >*>*>";
			   vp = "<*<*< NUKING >*>*>";	
			   v  = "<*<*< NUKE >*>*>"; }
    else if (dam <= 400) { vs = "++++ PLASMERISES ++++";
			   vp = "++++ PLASMERISING ++++";	
			   v  = "++++ PLASMERISE ++++"; }
    else if (dam <= 500) { vs = "***** TOTALLY DESTROYS *****";
			   vp = "***** TOTALLY DESTROYING *****";	
			   v  = "***** TOTALLY DESTROY *****"; }
    else 		 { vs = "--==>*>*> UTTERLY OBLITERATES <*<*<==--";
			   vp = "--==>*>*> UTTERLY OBLITERATING <*<*<==--"; 
			   v  = "--==>*>*> UTTERLY OBLITERATE <*<*<==--";}

    crash("Display weapon damage 4");

    /* hit with hilt */
    if ( atm < 15 )
    {
	if (wield->value[0] == WEAPON_POLEARM)
	{
	    // Myurr swings his polearm round, and clubs Jander with the handle.
	    sprintf(buf1, "$n swings $s %s round, and clubs $N with the handle.",weap);
	    sprintf(buf2, "$n swings $s %s round, and clubs you with the handle.", weap);
	    sprintf(buf3, "You swing your %s round, and club $N with the handle.",weap);
	}
	else if (wield->value[0] == WEAPON_SWORD)
 	{
	    // Myurr clubs Jander in the face with the hilt of his sword.
	    sprintf(buf1, "$n clubs $N in the face with the hilt of $s %s.", weap);
	    sprintf(buf2, "$n clubs you in the face with the hilt of $s %s.", weap);
	    sprintf(buf3, "You club $N in the face with the hilt of your %s.", weap);
	}
	else
 	{
	    sprintf(buf1, "$n clubs $N in the face with the handle of $s %s.", weap);
	    sprintf(buf2, "$n clubs you in the face with the handle of $s %s.", weap);
	    sprintf(buf3, "You club $N in the face with the handle of your %s.", weap);
	}
    	act(buf1, ch, NULL, victim, TO_NOTVICT);
    	act(buf2, ch, NULL, victim, TO_VICT);
    	act(buf3, ch, NULL, victim, TO_CHAR);
    }

    /* produce message in the form of:
	     Myurr's long sword cleaves Jander, MUTILATING him.
	     Myurr MUTILATES Jander with his long sword.
	     With his long sword, Myurr cleaves Jander, MUTILATING him.
	     Myurr cleaves Jander with his long sword, MUTILATING him.
    */
    mesg = dice(1, 4);
    if (mesg == 1)
    {
//	     Myurr's long sword cleaves Jander, MUTILATING him.
	sprintf(buf1, "$n's %s %s $N, %s $M.",weap,attacks,vp);
	sprintf(buf2, "$n's %s %s you, %s you.",weap,attacks,vp);
	sprintf(buf3, "Your %s %s $N, %s $M.",weap,attacks,vp);
    } else if (mesg == 2)
    {
//	     Myurr MUTILATES Jander with his long sword.
	sprintf(buf1, "$n %s $N with $s %s.",vs,weap);
	sprintf(buf2, "$n %s you with $s %s.",vs,weap);
	sprintf(buf3, "You %s $N with your %s.",v,weap);
    } else if (mesg == 3)
    {
//	     With his long sword, Myurr cleaves Jander, MUTILATING him.
	sprintf(buf1, "With $s %s, $n %s $N, %s $M.",weap,attacks,vp);
	sprintf(buf2, "With $s %s, $n %s you, %s you.",weap,attacks,vp);
	sprintf(buf3, "With your %s, you %s $N, %s $M.",weap,attack,vp);
    } else
    {
//	     Myurr cleaves Jander with his long sword, MUTILATING him.
	sprintf(buf1, "$n %s $N with $s %s, %s $M.",attacks, weap,vp);
	sprintf(buf2, "$n %s you with $s %s, %s you.",attacks, weap,vp);
	sprintf(buf3, "You %s $N with your %s, %s $M.",attack, weap,vp);
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

    if (atm < 2)
    {
	damage_obj( wield, dice(1, 10) + 10 );
	crash("just damaged the obj if atm < 2");
	act("You attack was so bad that you smash $p on the floor.", ch, wield, NULL, TO_CHAR);
    }
    else if (atm < 4)
    {
	damage_obj( wield, dice(1, 4) );
	crash("just damaged the obj if atm < 4");
	act("You attack so clumsily that you damage $p.", ch, wield, NULL, TO_CHAR);
    }
 
    crash("finding random armor to damage");

    if (atm > 98)
    {
	OBJ_DATA *armour;

	armour = get_random_armour( victim );
	if (armour != NULL)
	{
	    damage_obj( armour, dice(1, 4) + 4 );
	    sprintf(buf1, "$n's skillful attack badly damages $N's armour.");
	    sprintf(buf2, "$n's skillful attack badly damages your armour.");
	    sprintf(buf3, "Your skillful attack badly damages $N's armour.");

	    act(buf1, ch, NULL, victim, TO_NOTVICT);
	    act(buf2, ch, NULL, victim, TO_VICT);
	    act(buf3, ch, NULL, victim, TO_CHAR);
	}
    }
    else if (atm > 90)
    {
	OBJ_DATA *armour;

	armour = get_random_armour( victim );
	if (armour != NULL)
	{
	    damage_obj( armour, dice(1, 4) );
	    sprintf(buf1, "$n's skillful attack damages $N's armour.");
	    sprintf(buf2, "$n's skillful attack damages your armour.");
	    sprintf(buf3, "Your skillful attack damages $N's armour.");

	    act(buf1, ch, NULL, victim, TO_NOTVICT);
	    act(buf2, ch, NULL, victim, TO_VICT);
	    act(buf3, ch, NULL, victim, TO_CHAR);
	}
    }
    crash("end of display_weapon_dam");
}

void display_weapon_miss(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield)
{
    char buf1[256], buf2[256], buf3[256];
    const char *attack, *attacks;
    char *weap;
    int dt, mesg;

    if (ch == NULL || victim == NULL || wield == NULL)
	return;

    crash("displying weapons missing");

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';

    weap = wield->short_descr;
    if (weap[0] == 'a' && weap[1] == ' ')
	weap = &weap[2];

    dt = TYPE_HIT + wield->value[3];
    if (dt < TYPE_HIT)
    {
    	attack = attack_table[wield->value[3]].noun;
    	attacks = attack_table[wield->value[3]].noun_p;
    }
    else
    {
	attack = attack_table[dt - TYPE_HIT].noun;
	attacks = attack_table[dt - TYPE_HIT].noun_p;
    }

    mesg = dice(1,5);

    if (mesg == 1)
    {
//		Myurr slashes at Jander with his long sword, but misses him.
	sprintf(buf1, "$n %s at $N with $s %s, but misses $M.",attacks,weap);
	sprintf(buf2, "$n %s at you with $s %s, but $e misses.",attacks,weap);
	sprintf(buf3, "You %s at $N with your %s, but miss.",attack,weap);
    } else if (mesg == 2)
    {
//		Myurr's long sword glances Jander's armour.
	sprintf(buf1,"$n's %s glances $N's armour.",weap);
	sprintf(buf2,"$n's %s glances your armour.",weap);
	sprintf(buf3,"Your %s glances $N's armour.",weap);
    } else if (mesg == 3)
    {
//		Myurr's slash narrowly misses Jander.
	sprintf(buf1,"$n's %s narrowly misses $N.",attack);
	sprintf(buf2,"$n's %s narrowly misses you.",attack);
	sprintf(buf3,"Your %s narrowly misses $N.",attack);
    } else if (mesg == 4)
    {
//		Myurr slashes wildly at Jander with his long sword.
	sprintf(buf1,"$n %s wildly at $N with $s %s.",attacks,weap);
	sprintf(buf2,"$n %s wildly at you with $s %s.",attacks,weap);
	sprintf(buf3,"You %s wildly at $N with your %s.",attack,weap);
    } else if (mesg == 5)
    {
//		With his long sword, Myurr slashes at Jander to no avail.
	sprintf(buf1,"With $s %s, $n %s at $N to no avail.",weap,attacks);
	sprintf(buf2,"With $s %s, $n %s at you to no avail.",weap,attacks);
	sprintf(buf3,"With your %s, you %s at $N to no avail.",weap,attack);
    }
    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

    if (dice(1, 100) < 5)
    {
	crash("hmmm, a call to damage obj here");
	damage_obj( wield, dice(1, 4) );
	act("You attack so clumsily that you damage $p.", ch, wield, NULL, TO_CHAR);
	act("$n attacks so clumsily that $e damages $p.", ch, wield, NULL, TO_ROOM);
    }
}

void display_punch_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    const char *vs, *vp, *v;
    int mesg;

    if (ch == NULL || victim == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch != victim)
	{
	    sprintf(buf1,"#2$n's attack passes harmlessly through you.");
	    sprintf(buf2,"#2Your attack passes harmlessly through $N.");
	    sprintf(buf3,"#2$n's attack passes harlessly through you.");
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own attack.");
	    sprintf(buf2,"#2Luckily, you are immune to that.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's attack!");
	    sprintf(buf2,"#2$N is unaffected by your attack!");
	    sprintf(buf3,"#2$n's attack is powerless against you.");
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }
    else if (dam <=   4) { vs = "bruises";	vp = "bruising";	
			   v  = "bruise"; }
    else if (dam <=   6) { vs = "scratches";	vp = "scratching";	
			   v  = "scratch"; }
    else if (dam <=   8) { vs = "grazes";	vp = "grazing";		
			   v  = "graze"; }
    else if (dam <=  12) { vs = "hits";		vp = "hitting";		
			   v  = "hit"; }
    else if (dam <=  16) { vs = "injures";	vp = "injuring";	
			   v  = "injure"; }
    else if (dam <=  20) { vs = "wounds";	vp = "wounding";	
			   v  = "wound"; }
    else if (dam <=  24) { vs = "mauls";	vp = "mauling";		
			   v  = "maul"; }
    else if (dam <=  28) { vs = "decimates";	vp = "decimating";	
			   v  = "decimate"; }
    else if (dam <=  32) { vs = "devastates";	vp = "devastating";	
			   v  = "devastate"; }
    else if (dam <=  36) { vs = "maims";	vp = "maiming";	
			   v  = "maim"; }
    else if (dam <=  42) { vs = "MUTILATES";	vp = "MUTILATING";	
			   v  = "MUTILATE"; }
    else if (dam <=  48) { vs = "DISEMBOWELS";	vp = "DISEMBOWELING";	
			   v  = "DISEMBOWEL"; }
    else if (dam <=  54) { vs = "DISMEMBERS";	vp = "DISMEMBERING";	
			   v  = "DISMEMBER"; }
    else if (dam <=  60) { vs = "MASSACRES";	vp = "MASSACRING";	
			   v  = "MASSACRE"; }
    else if (dam <=  68) { vs = "MANGLES";	vp = "MANGLING";	
			   v  = "MANGLE"; }
    else if (dam <=  76) { vs = "> CRUSHES <";	vp = "> CRUSHING <";	
			   v  = "> CRUSH <"; }
    else if (dam <=  86) { vs = "+- DESECRATES -+";
			   vp = "+- DESECRATING -+";	
			   v  = "+- DESECRATE -+"; }
    else if (dam <= 100) { vs = ">- DEMOLISHES -<";
			   vp = ">- DEMOLISHING -<";	
			   v  = ">- DEMOLISH -<"; }
    else if (dam <= 110) { vs = "*** DEVASTATES ***";
			   vp = "*** DEVASTATING ***";	
			   v  = "*** DEVASTATE ***"; }
    else if (dam <= 130) { vs = "*** EVISCERATES ***";
			   vp = "*** EVISCERATING ***";	
			   v  = "*** EVISCERATE ***"; }
    else if (dam <= 160) { vs = "=== OBLITERATES ===";
			   vp = "=== OBLITERATING ===";	
			   v  = "=== OBLITERATE ==="; }
    else if (dam <= 200) { vs = ">>> ANNIHILATES <<<";
			   vp = ">>> ANNIHILATING <<<";	
			   v  = ">>> ANNIHILATE <<<"; }
    else if (dam <= 240) { vs = "<<< ERADICATES >>>";
			   vp = "<<< ERADICATING >>>";	
			   v  = "<<< ERADICATE >>>"; }
    else if (dam <= 280) { vs = ">-> LIQUEFIES <-<";
			   vp = ">-> LIQUEFYING <-<";	
			   v  = ">-> LIQUEFY <-<"; }
    else if (dam <= 320) { vs = "ooo PULVERISES ooo";
			   vp = "ooo PULVERISING ooo";	
			   v  = "ooo PULVERISE ooo"; }
    else if (dam <= 360) { vs = "<*<*< NUKES >*>*>";
			   vp = "<*<*< NUKING >*>*>";	
			   v  = "<*<*< NUKE >*>*>"; }
    else if (dam <= 400) { vs = "++++ PLASMERISES ++++";
			   vp = "++++ PLASMERISING ++++";	
			   v  = "++++ PLASMERISE ++++"; }
    else if (dam <= 500) { vs = "***** TOTALLY DESTROYS *****";
			   vp = "***** TOTALLY DESTROYING *****";	
			   v  = "***** TOTALLY DESTROY *****"; }
    else 		 { vs = "--==>*>*> UTTERLY OBLITERATES <*<*<==--";
			   vp = "--==>*>*> UTTERLY OBLITERATING <*<*<==--"; 
			   v  = "--==>*>*> UTTERLY OBLITERATE <*<*<==--";}



	if (IS_SET(ch->flag2, AFF_CLAW) )
	{
		// Myurrs claws tear at Janders flesh, mauling him.
		// Myurr mauls Jander with his claws.
		// With his claws, Myurr mauls Jander.
	    mesg = dice(1, 3);
	    if (mesg == 1)
	    {
		// Myurrs claws tear at Janders flesh, mauling him.
		sprintf(buf1,"$n's claws tear at $N's flesh, %s $M.",vp);
		sprintf(buf2,"$n's claws tear at your flesh, %s you.",vp);
		sprintf(buf3,"Your claws tear at $N's flesh, %s $M.",vp);
	    }
	    else if (mesg == 2)
	    {
		// Myurr mauls Jander with his claws.
		sprintf(buf1,"$n %s $N with $s claws.",vs);
		sprintf(buf2,"$n %s you with $s claws.",vs);
		sprintf(buf3,"You %s $N with your claws.",v);
	    }
	    else if (mesg == 3)
	    {
		// With his claws, Myurr mauls Jander.
		sprintf(buf1,"With $s claws, $n %s $N.",vs);
		sprintf(buf2,"With $s claws, $n %s you.",vs);
		sprintf(buf3,"With your claws, you %s $N.",v);
	    }
	}
	else if (ch->cur_form == FRM_CRIMSON)
	{
	    mesg = dice(1, 3);
	    if (mesg == 1)
	    {
		sprintf(buf1,"$n's tentacles suck at $N's lifeforce, %s $M.",vp);
		sprintf(buf2,"$n's tentacles suck at your lifeforce, %s you.",vp);
		sprintf(buf3,"Your tentacles suck at $N's lifeforce, %s $M.",vp);
	    }
	    else if (mesg == 2)
	    {
		sprintf(buf1,"$n %s $N with $s draining.",vs);
		sprintf(buf2,"$n %s you with $s draining.",vs);
		sprintf(buf3,"You %s $N with your draining.",v);
	    }
	    else if (mesg == 3)
	    {
		sprintf(buf1,"With $s flailing tentacles, $n %s $N.",vs);
		sprintf(buf2,"With $s flailing tentacles, $n %s you.",vs);
		sprintf(buf3,"With your flailing tentacles, you %s $N.",v);
	    }
	}
	else if (ch->cur_form == FRM_WHIRLWIND)
	{
	    mesg = dice(1, 3);
	    if (mesg == 1)
	    {
		sprintf(buf1,"$n spins $N around, %s $M.",vp);
		sprintf(buf2,"$n spins you around, %s you.",vp);
		sprintf(buf3,"You spin $N around, %s $M.",vp);
	    }
	    else if (mesg == 2)
	    {
		sprintf(buf1,"$n %s $N with $s blast of air.",vs);
		sprintf(buf2,"$n %s you with $s blast of air.",vs);
		sprintf(buf3,"You %s $N with your blast of air.",v);
	    }
	    else if (mesg == 3)
	    {
		sprintf(buf1,"With $s air blast, $n %s $N.",vs);
		sprintf(buf2,"With $s air blast, $n %s you.",vs);
		sprintf(buf3,"With your air blast, you %s $N.",v);
	    }
	}
	else if (ch->cur_form == FRM_FLAME)
	{
	    mesg = dice(1, 3);
	    if (mesg == 1)
	    {
		sprintf(buf1,"$n envelops $N in $s furnace, %s $M.",vp);
		sprintf(buf2,"$n envelops you in $s furnace, %s you.",vp);
		sprintf(buf3,"You envelop $N in your furnace, %s $M.",vp);
	    }
	    else if (mesg == 2)
	    {
		sprintf(buf1,"$n %s $N with $s blast of flame.",vs);
		sprintf(buf2,"$n %s you with $s blast of flame.",vs);
		sprintf(buf3,"You %s $N with your blast of flame.",v);
	    }
	    else if (mesg == 3)
	    {
		sprintf(buf1,"With $s flame blast, $n %s $N.",vs);
		sprintf(buf2,"With $s flame blast, $n %s you.",vs);
		sprintf(buf3,"With your flame blast, you %s $N.",v);
	    }
	}
	else if (ch->cur_form == FRM_LIGHT)
	{
	    mesg = dice(1, 3);
	    if (mesg == 1)
	    {
		sprintf(buf1,"$n envelops $N in $s radiance, %s $M.",vp);
		sprintf(buf2,"$n envelops you in $s radiance, %s you.",vp);
		sprintf(buf3,"You envelop $N in your radiance, %s $M.",vp);
	    }
	    else if (mesg == 2)
	    {
		sprintf(buf1,"$n %s $N with $s beam of light.",vs);
		sprintf(buf2,"$n %s you with $s beam of light.",vs);
		sprintf(buf3,"You %s $N with your beam of light.",v);
	    }
	    else if (mesg == 3)
	    {
		sprintf(buf1,"With $s light, $n %s $N.",vs);
		sprintf(buf2,"With $s light, $n %s you.",vs);
		sprintf(buf3,"With your light, you %s $N.",v);
	    }
	}
	else
	{
	    mesg = dice(1, 4);
	    if (mesg == 1)
	    {
		// Myurr punches Jander, mauling him.
	    	sprintf(buf1,"$n punches $N, %s him.",vp);
	    	sprintf(buf2,"$n punches you, %s you.",vp);
	    	sprintf(buf3,"You punch $N, %s him.",vp);
	    }
	    else if (mesg == 2)
	    {
		// Myurrs uppercut mauls Jander.
		sprintf(buf1,"$n's uppercut %s $N.", vs);
		sprintf(buf2,"$n's uppercut %s you.", vs);
		sprintf(buf3,"Your uppercut %s $N.", vs);
	    }
	    else if (mesg == 3)
	    {
		// Myurr blow mauls Jander.
		sprintf(buf1,"$n's blow %s $N.",vs);
		sprintf(buf2,"$n's blow %s you.",vs);
		sprintf(buf3,"Your blow %s $N.",vs);
	    }
	    else if (mesg == 4)
	    {
		// Jander spins round and punches Myurr
		sprintf(buf1,"$n spins around and punches $N %s $M.",vp);
		sprintf(buf2,"$n spins around and punches you, %s you.",vp);
		sprintf(buf3,"You spin around and punch $N, %s $M.",vp);
	    }
	}

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

}


void display_kick_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    const char *vs, *vp, *v;
    int mesg;

    if (ch == NULL || victim == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch != victim)
	{
	    sprintf(buf1,"#2$n's attack passes harmlessly through you.");
	    sprintf(buf2,"#2Your attack passes harmlessly through $N.");
	    sprintf(buf3,"#2$n's attack passes harlessly through you.");
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own attack.");
	    sprintf(buf2,"#2Luckily, you are immune to that.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's attack!");
	    sprintf(buf2,"#2$N is unaffected by your attack!");
	    sprintf(buf3,"#2$n's attack is powerless against you.");
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }
    else if (dam <=   4) { vs = "bruises";	vp = "bruising";	
			   v  = "bruise"; }
    else if (dam <=   6) { vs = "scratches";	vp = "scratching";	
			   v  = "scratch"; }
    else if (dam <=   8) { vs = "grazes";	vp = "grazing";		
			   v  = "graze"; }
    else if (dam <=  12) { vs = "hits";		vp = "hitting";		
			   v  = "hit"; }
    else if (dam <=  16) { vs = "injures";	vp = "injuring";	
			   v  = "injure"; }
    else if (dam <=  20) { vs = "wounds";	vp = "wounding";	
			   v  = "wound"; }
    else if (dam <=  24) { vs = "mauls";	vp = "mauling";		
			   v  = "maul"; }
    else if (dam <=  28) { vs = "decimates";	vp = "decimating";	
			   v  = "decimate"; }
    else if (dam <=  32) { vs = "devastates";	vp = "devastating";	
			   v  = "devastate"; }
    else if (dam <=  36) { vs = "maims";	vp = "maiming";	
			   v  = "maim"; }
    else if (dam <=  42) { vs = "MUTILATES";	vp = "MUTILATING";	
			   v  = "MUTILATE"; }
    else if (dam <=  48) { vs = "DISEMBOWELS";	vp = "DISEMBOWELING";	
			   v  = "DISEMBOWEL"; }
    else if (dam <=  54) { vs = "DISMEMBERS";	vp = "DISMEMBERING";	
			   v  = "DISMEMBER"; }
    else if (dam <=  60) { vs = "MASSACRES";	vp = "MASSACRING";	
			   v  = "MASSACRE"; }
    else if (dam <=  68) { vs = "MANGLES";	vp = "MANGLING";	
			   v  = "MANGLE"; }
    else if (dam <=  76) { vs = "> CRUSHES <";	vp = "> CRUSHING <";	
			   v  = "> CRUSH <"; }
    else if (dam <=  86) { vs = "+- DESECRATES -+";
			   vp = "+- DESECRATING -+";	
			   v  = "+- DESECRATE -+"; }
    else if (dam <= 100) { vs = ">- DEMOLISHES -<";
			   vp = ">- DEMOLISHING -<";	
			   v  = ">- DEMOLISH -<"; }
    else if (dam <= 110) { vs = "*** DEVASTATES ***";
			   vp = "*** DEVASTATING ***";	
			   v  = "*** DEVASTATE ***"; }
    else if (dam <= 130) { vs = "*** EVISCERATES ***";
			   vp = "*** EVISCERATING ***";	
			   v  = "*** EVISCERATE ***"; }
    else if (dam <= 160) { vs = "=== OBLITERATES ===";
			   vp = "=== OBLITERATING ===";	
			   v  = "=== OBLITERATE ==="; }
    else if (dam <= 200) { vs = ">>> ANNIHILATES <<<";
			   vp = ">>> ANNIHILATING <<<";	
			   v  = ">>> ANNIHILATE <<<"; }
    else if (dam <= 240) { vs = "<<< ERADICATES >>>";
			   vp = "<<< ERADICATING >>>";	
			   v  = "<<< ERADICATE >>>"; }
    else if (dam <= 280) { vs = ">-> LIQUEFIES <-<";
			   vp = ">-> LIQUEFYING <-<";	
			   v  = ">-> LIQUEFY <-<"; }
    else if (dam <= 320) { vs = "ooo PULVERISES ooo";
			   vp = "ooo PULVERISING ooo";	
			   v  = "ooo PULVERISE ooo"; }
    else if (dam <= 360) { vs = "<*<*< NUKES >*>*>";
			   vp = "<*<*< NUKING >*>*>";	
			   v  = "<*<*< NUKE >*>*>"; }
    else if (dam <= 400) { vs = "++++ PLASMERISES ++++";
			   vp = "++++ PLASMERISING ++++";	
			   v  = "++++ PLASMERISE ++++"; }
    else if (dam <= 500) { vs = "***** TOTALLY DESTROYS *****";
			   vp = "***** TOTALLY DESTROYING *****";	
			   v  = "***** TOTALLY DESTROY *****"; }
    else 		 { vs = "--==>*>*> UTTERLY OBLITERATES <*<*<==--";
			   vp = "--==>*>*> UTTERLY OBLITERATING <*<*<==--"; 
			   v  = "--==>*>*> UTTERLY OBLITERATE <*<*<==--";}


    mesg = dice(1, get_size(ch) >= SIZE_LARGE ? 2 : 3);
    sprintf(buf1,"$n kicks $N, %s $M.",vp);
    sprintf(buf2,"$n kicks you, %s you.",vp);
    sprintf(buf3,"You kick $N, %s $M.",vp);

    if (mesg == 2)
    {
	// Jander sweeps you down and kicks you in the side
	sprintf(buf1,"$n sweeps $N and kicks $M in the side, %s $M.",vp);
    	sprintf(buf2,"$n sweeps you down and kicks you in the side, %s you.",vp);
    	sprintf(buf3,"You sweep $N down and kick $M in the side, %s $M.",vp);
    }
    else
    {
    	sprintf(buf1,"$n leaps up and kicks $N in the chest, %s $M.",vp);
    	sprintf(buf2,"$n leaps up and kicks you in the chest, %s you.",vp);
    	sprintf(buf3,"You leap up and kick $N in the chest, %s $M.",vp);
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

}


void display_bash_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    const char *vs, *vp, *v;
    int mesg;

    if (ch == NULL || victim == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch != victim)
	{
	    sprintf(buf1,"#2$n's attack passes harmlessly through you.");
	    sprintf(buf2,"#2Your attack passes harmlessly through $N.");
	    sprintf(buf3,"#2$n's attack passes harlessly through you.");
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own attack.");
	    sprintf(buf2,"#2Luckily, you are immune to that.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's attack!");
	    sprintf(buf2,"#2$N is unaffected by your attack!");
	    sprintf(buf3,"#2$n's attack is powerless against you.");
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }
    else if (dam <=   4) { vs = "bruises";	vp = "bruising";	
			   v  = "bruise"; }
    else if (dam <=   6) { vs = "scratches";	vp = "scratching";	
			   v  = "scratch"; }
    else if (dam <=   8) { vs = "grazes";	vp = "grazing";		
			   v  = "graze"; }
    else if (dam <=  12) { vs = "hits";		vp = "hitting";		
			   v  = "hit"; }
    else if (dam <=  16) { vs = "injures";	vp = "injuring";	
			   v  = "injure"; }
    else if (dam <=  20) { vs = "wounds";	vp = "wounding";	
			   v  = "wound"; }
    else if (dam <=  24) { vs = "mauls";	vp = "mauling";		
			   v  = "maul"; }
    else if (dam <=  28) { vs = "decimates";	vp = "decimating";	
			   v  = "decimate"; }
    else if (dam <=  32) { vs = "devastates";	vp = "devastating";	
			   v  = "devastate"; }
    else if (dam <=  36) { vs = "maims";	vp = "maiming";	
			   v  = "maim"; }
    else if (dam <=  42) { vs = "MUTILATES";	vp = "MUTILATING";	
			   v  = "MUTILATE"; }
    else if (dam <=  48) { vs = "DISEMBOWELS";	vp = "DISEMBOWELING";	
			   v  = "DISEMBOWEL"; }
    else if (dam <=  54) { vs = "DISMEMBERS";	vp = "DISMEMBERING";	
			   v  = "DISMEMBER"; }
    else if (dam <=  60) { vs = "MASSACRES";	vp = "MASSACRING";	
			   v  = "MASSACRE"; }
    else if (dam <=  68) { vs = "MANGLES";	vp = "MANGLING";	
			   v  = "MANGLE"; }
    else if (dam <=  76) { vs = "> CRUSHES <";	vp = "> CRUSHING <";	
			   v  = "> CRUSH <"; }
    else if (dam <=  86) { vs = "+- DESECRATES -+";
			   vp = "+- DESECRATING -+";	
			   v  = "+- DESECRATE -+"; }
    else if (dam <= 100) { vs = ">- DEMOLISHES -<";
			   vp = ">- DEMOLISHING -<";	
			   v  = ">- DEMOLISH -<"; }
    else if (dam <= 110) { vs = "*** DEVASTATES ***";
			   vp = "*** DEVASTATING ***";	
			   v  = "*** DEVASTATE ***"; }
    else if (dam <= 130) { vs = "*** EVISCERATES ***";
			   vp = "*** EVISCERATING ***";	
			   v  = "*** EVISCERATE ***"; }
    else if (dam <= 160) { vs = "=== OBLITERATES ===";
			   vp = "=== OBLITERATING ===";	
			   v  = "=== OBLITERATE ==="; }
    else if (dam <= 200) { vs = ">>> ANNIHILATES <<<";
			   vp = ">>> ANNIHILATING <<<";	
			   v  = ">>> ANNIHILATE <<<"; }
    else if (dam <= 240) { vs = "<<< ERADICATES >>>";
			   vp = "<<< ERADICATING >>>";	
			   v  = "<<< ERADICATE >>>"; }
    else if (dam <= 280) { vs = ">-> LIQUEFIES <-<";
			   vp = ">-> LIQUEFYING <-<";	
			   v  = ">-> LIQUEFY <-<"; }
    else if (dam <= 320) { vs = "ooo PULVERISES ooo";
			   vp = "ooo PULVERISING ooo";	
			   v  = "ooo PULVERISE ooo"; }
    else if (dam <= 360) { vs = "<*<*< NUKES >*>*>";
			   vp = "<*<*< NUKING >*>*>";	
			   v  = "<*<*< NUKE >*>*>"; }
    else if (dam <= 400) { vs = "++++ PLASMERISES ++++";
			   vp = "++++ PLASMERISING ++++";	
			   v  = "++++ PLASMERISE ++++"; }
    else if (dam <= 500) { vs = "***** TOTALLY DESTROYS *****";
			   vp = "***** TOTALLY DESTROYING *****";	
			   v  = "***** TOTALLY DESTROY *****"; }
    else 		 { vs = "--==>*>*> UTTERLY OBLITERATES <*<*<==--";
			   vp = "--==>*>*> UTTERLY OBLITERATING <*<*<==--"; 
			   v  = "--==>*>*> UTTERLY OBLITERATE <*<*<==--";}



	    mesg = dice(1, 1);
	    if (mesg == 1)
	    {
		// Myurr punches Jander, mauling him.
	    	sprintf(buf1,"$n bashes into $N, %s $M and forcing $M back.",vp);
	    	sprintf(buf2,"$n bashes into you, %s you and forcing you back.",vp);
	    	sprintf(buf3,"You bash into $N, %s $M and forcing $M back.",vp);
	    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

}

void display_bite_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    int mesg;

    if (ch == NULL || victim == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch != victim)
	{
	    sprintf(buf1,"#2$n's attack passes harmlessly through you.");
	    sprintf(buf2,"#2Your attack passes harmlessly through $N.");
	    sprintf(buf3,"#2$n's attack passes harlessly through you.");
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own attack.");
	    sprintf(buf2,"#2Luckily, you are immune to that.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's attack!");
	    sprintf(buf2,"#2$N is unaffected by your attack!");
	    sprintf(buf3,"#2$n's attack is powerless against you.");
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }

	    mesg = dice(1, 1);
	    if (mesg == 1)
	    {
	    	sprintf(buf1,"$n lunges forwards and bites $N, %s $M.",damtext_vp(dam));
	    	sprintf(buf2,"$n lunges forwards and bites you, %s you.",damtext_vp(dam));
	    	sprintf(buf3,"You lunge forwards and bite $N, %s $M.",damtext_vp(dam));
	    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

}

void display_tail_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    int mesg;

    if (ch == NULL || victim == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch != victim)
	{
	    sprintf(buf1,"#2$n's attack passes harmlessly through you.");
	    sprintf(buf2,"#2Your attack passes harmlessly through $N.");
	    sprintf(buf3,"#2$n's attack passes harlessly through you.");
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own attack.");
	    sprintf(buf2,"#2Luckily, you are immune to that.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's attack!");
	    sprintf(buf2,"#2$N is unaffected by your attack!");
	    sprintf(buf3,"#2$n's attack is powerless against you.");
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }

	    mesg = dice(1, 1);
	    if (mesg == 1)
	    {
	    	sprintf(buf1,"$n smashes $s tail into $N, %s $M.",damtext_vp(dam));
	    	sprintf(buf2,"$n smashes $s tail into you, %s you.",damtext_vp(dam));
	    	sprintf(buf3,"You smash your tail into $N, %s $M.",damtext_vp(dam));
	    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

}

void display_wing_damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
    char buf1[256], buf2[256], buf3[256], buf[256];
    int mesg;

    if (ch == NULL || victim == NULL)
	return;

    buf1[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';
    buf[0]  = '\0';

    if (IS_SET(victim->in_room->room_flags, ROOM_ETHERIAL))
    {
	if (ch != victim)
	{
	    sprintf(buf1,"#2$n's attack passes harmlessly through you.");
	    sprintf(buf2,"#2Your attack passes harmlessly through $N.");
	    sprintf(buf3,"#2$n's attack passes harlessly through you.");
	    act(buf1, ch, NULL, victim, TO_VICT);
	    act(buf2, ch, NULL, victim, TO_CHAR);
	    act(buf3, ch, NULL, victim, TO_NOTVICT);
	}
	return;
    }

    if (dam == 0)
    {
	if (ch == victim)
	{
	    sprintf(buf1,"#2$n is unaffected by $s own attack.");
	    sprintf(buf2,"#2Luckily, you are immune to that.");
	    act(buf1, ch, NULL, victim, TO_ROOM);
	    stc(buf2, ch);
	}
	else
	{
	    sprintf(buf1,"#2$N is unaffected by $n's attack!");
	    sprintf(buf2,"#2$N is unaffected by your attack!");
	    sprintf(buf3,"#2$n's attack is powerless against you.");
	    act(buf1,ch,NULL,victim,TO_NOTVICT);
	    act(buf2,ch,NULL,victim,TO_CHAR);
	    act(buf3,ch,NULL,victim,TO_VICT);
	}
	return;
    }

	    mesg = dice(1, 1);
	    if (mesg == 1)
	    {
	    	sprintf(buf1,"$n's wing buffet %s $N.",damtext_vs(dam));
	    	sprintf(buf2,"$n's wing buffet %s you.",damtext_vs(dam));
	    	sprintf(buf3,"Your wing buffet %s $N",damtext_vs(dam));
	    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);

}

void display_bite_miss(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf1[256], buf2[256], buf3[256];
    int mesg;

    // Myurr swings wildly at Jander but misses.
    // Jander ducks Myurrs blow.

    mesg = dice(1, 1);
    if (mesg == 1)
    {
	// Myurr swings wildly at Jander but misses.
	sprintf(buf1, "$n lunges forwards to bite $N but misses.");
	sprintf(buf2, "$n lunges forwards to bite you but misses.");
	sprintf(buf3, "You lunge forwards to bite $N but miss.");
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);
}

void display_tail_miss(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf1[256], buf2[256], buf3[256];
    int mesg;

    mesg = dice(1, 1);
    if (mesg == 1)
    {
	sprintf(buf1, "$n swings $s tail at $N but misses.");
	sprintf(buf2, "$n swings $s tail at you but misses.");
	sprintf(buf3, "You swing your tail at $N but miss.");
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);
}

void display_wing_miss(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf1[256], buf2[256], buf3[256];
    int mesg;

    mesg = dice(1, 1);
    if (mesg == 1)
    {
	sprintf(buf1, "$n's wing buffet misses $N.");
	sprintf(buf2, "$n's wing buffet misses you.");
	sprintf(buf3, "Your wing buffet misses $N.");
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);
}

void display_punch_miss(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf1[256], buf2[256], buf3[256];
    int mesg;

    // Myurr swings wildly at Jander but misses.
    // Jander ducks Myurrs blow.

    mesg = dice(1, 2);
    if (mesg == 1)
    {
	// Myurr swings wildly at Jander but misses.
	sprintf(buf1, "$n swings wildly at $N but misses.");
	sprintf(buf2, "$n swings wildly at you but misses.");
	sprintf(buf3, "You swing wildly at $N but miss.");
    } else if (mesg == 2)
    {
    	// Jander ducks Myurrs blow.
	sprintf(buf1, "$N ducks $n's blow.");
	sprintf(buf2, "You duck $n's blow.");
	sprintf(buf3, "$N ducks your blow.");
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);
}
void display_kick_miss(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf1[256], buf2[256], buf3[256];
    int mesg;

    mesg = dice(1, 3);
    if (mesg == 1 || 1 == 1)
    {
	sprintf(buf1, "$n kicks wildly at $N but misses.");
	sprintf(buf2, "$n kicks wildly at you but misses.");
	sprintf(buf3, "You kick wildly at $N but miss.");
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);
}

void display_bash_miss(CHAR_DATA *ch, CHAR_DATA *victim)
{
    char buf1[256], buf2[256], buf3[256];
    int mesg;

    mesg = dice(1, 3);
    if (mesg == 1)
    {
	sprintf(buf1, "$n tries to bash into $N but misses.");
	sprintf(buf2, "$n tries to bash into you but misses.");
	sprintf(buf3, "You try to bash into $N but miss.");
    }

    act(buf1, ch, NULL, victim, TO_NOTVICT);
    act(buf2, ch, NULL, victim, TO_VICT);
    act(buf3, ch, NULL, victim, TO_CHAR);
}

bool check_shield_block(CHAR_DATA *ch, CHAR_DATA *victim)
{
    OBJ_DATA *hand;
    int chance;
    int arm;

    crash("checking shield block");

    arm = get_block_arm( ch, ITEM_ARMOR );
    if (arm == -1) return FALSE;
    if (ch->initiative[arm] <= 0)
        return FALSE;

    hand = get_eq_char(ch, body_arm(arm));
    if (hand->item_type != ITEM_ARMOR && hand->item_type != ITEM_WEAPON)
        hand = NULL;
    if (hand == NULL)
        return FALSE;

    crash("checking shield block 2");

    ch->initiative[arm] -= 1;

    chance = get_skill(ch, gsn_shield_block);
    if (dice(1, 100) < chance)
    {
        act("$n blocks $N's blow with $m shield.", ch, NULL, victim, TO_NOTVICT);
        act("$n blocks your blow with $m shield.", ch, NULL, victim, TO_VICT);
        act("You block $N's blow with your shield.", ch, NULL, victim, TO_CHAR);
        return TRUE;
    }
    else
    {
        check_improve(ch, gsn_shield_block, FALSE, 5);
        act("$n tries to block $N's blow with $m shield, but fails.", ch, NULL, victim, TO_NOTVICT);
        act("$n tries to block your blow with $m shield, but fails.", ch, NULL, victim, TO_VICT);
        act("You try to block $N's blow with your shield, but fail.", ch, NULL, victim, TO_CHAR);
    }
    crash("checking shield block 3");
 
    return FALSE;
}

bool check_dodge(CHAR_DATA *ch, CHAR_DATA *victim)
{
    int sk = get_skill( ch, gsn_dodge ) * 3 / 2;
    int mesg = dice(1, 4);

    crash("check dodge function");

    if (ch->initiative[4] <= 0)
	return FALSE;
    ch->initiative[4] -= 1;
    if ( dice(1, 100) < sk )
    {
	if (mesg == 1)
	{
	    // Myurr ducks Janders attack.
	    act("$n ducks $N's attack.", ch, NULL, victim, TO_NOTVICT);
	    act("$n ducks your attack.", ch, NULL, victim, TO_VICT);
	    act("You duck $N's attack.", ch, NULL, victim, TO_CHAR);
	}
	else if (mesg == 2)
	{
	    // Myurr rolls out the way of Janders attack.
	    act("$n rolls out the way of $N's attack.", ch, NULL, victim, TO_NOTVICT);
	    act("$n rolls out the way of your attack.", ch, NULL, victim, TO_VICT);
	    act("You roll out the way of $N's attack.", ch, NULL, victim, TO_CHAR);
	}
	else if (mesg == 3)
	{
	    // Myurr dodges Janders attack.
	    act("$n dodges $N's attack.", ch, NULL, victim, TO_NOTVICT);
	    act("$n dodges your attack.", ch, NULL, victim, TO_VICT);
	    act("You dodge $N's attack.", ch, NULL, victim, TO_CHAR);
	}
	else 
	{
	    // Myurr dives out the way of Janders attack.	
	    act("$n dives out the way of $N's attack.", ch, NULL, victim, TO_NOTVICT);
	    act("$n dives out the way of your attack.", ch, NULL, victim, TO_VICT);
	    act("You dive out the way of $N's attack.", ch, NULL, victim, TO_CHAR);
	}
	return TRUE;
    }
    else
	check_improve(ch, gsn_parry, FALSE, 5);

    return FALSE;
}

bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim, int penalty )
{
    OBJ_DATA *hand;
    int chance;
    int arm;

    crash("checking parry");

    arm = get_block_arm( ch, ITEM_WEAPON );
    if (arm == -1) return FALSE;
    if (ch->initiative[arm] <= 0)
	return FALSE;

    hand = get_eq_char(ch, body_arm(arm));
    if (hand->item_type != ITEM_WEAPON && hand->item_type != ITEM_ARMOR)
    	hand = NULL;
    if (hand == NULL && !IS_SET(ch->flag4, AFF_STEEL_CLAWS))
    	return FALSE;

    crash("checking parry 2");

    ch->initiative[arm] -= 1;

    if (penalty == 2) penalty = 1;

    chance = get_skill(ch, gsn_parry) / 2 - penalty;
    chance += get_skill(ch, (hand == NULL) ? gsn_hand_to_hand : get_weapon_sn_by_type(hand));
    if (dice(1, 100) < chance)
    {
        act("$n parries $N's blow.", ch, NULL, victim, TO_NOTVICT);
        act("$n parries your blow.", ch, NULL, victim, TO_VICT);
        act("You parry $N's blow.", ch, NULL, victim, TO_CHAR);
	if (ch->secondaryrace == RACE_HERO
	    && ch->power[DISC_HERO_BATT] > 4
            && !IS_SET(ch->fight_style, FS_BLOCK))
	{
	    chance = 25 + (ch->power[DISC_HERO_BATT] * 5);
	    if (dice(1,100) < chance)
            {
		act("$n skillfully ripostes $N's attack.", ch, NULL, victim, TO_NOTVICT);
		act("$n skillfully ripostes your attack.", ch, NULL, victim, TO_VICT);
		act("You skillfully riposte $N's attack.", ch, NULL, victim, TO_CHAR);
                perform_hit(ch, victim, hand, FALSE);
 	    }
	}
        return TRUE;
    }
    else
    {
        check_improve(ch, gsn_parry, FALSE, 5);
        act("$n tries to parry $N's blow, but misses.", ch, NULL, victim, TO_NOTVICT);
        act("$n tries to parry your blow, but misses.", ch, NULL, victim, TO_VICT);
        act("You try to parry $N's blow, but miss.", ch, NULL, victim, TO_CHAR);
    }
    crash("checking parry 3");

    return FALSE;
}

int get_block_arm( CHAR_DATA *ch, int itemtype )
{
    OBJ_DATA *hand;
    int fs, arm;

    fs = (ch->fight_style == 0) ? (FS_CONSERVATIVE + FS_ANY_BLOCK): ch->fight_style;

    if (IS_SET(fs, FS_ATTACK)) return -1;

    if (ch->block_arm > -1)
    {
        if (ch->initiative[ch->block_arm] >= 2)
        {
            if ((hand = get_eq_char(ch, body_arm(ch->block_arm))) != NULL
            	 && (hand->item_type == itemtype))
	    return ch->block_arm;
	    if (IS_SET(ch->flag4, AFF_STEEL_CLAWS) && itemtype != ITEM_ARMOR)
		return ch->block_arm;
	}
    }

// if flagged, find highest or lowest

// find other

    for (arm = 0; arm < 4; arm++)
    {
        if (ch->initiative[arm] >= 2)
        {
            if ((hand = get_eq_char(ch, body_arm(arm))) != NULL
            	 && (hand->item_type == itemtype))
	    return arm;
	    if (IS_SET(ch->flag4, AFF_STEEL_CLAWS) && itemtype != ITEM_ARMOR)
		return arm;
	}
    }
    return -1;
}

int body_arm( int arm )
{
    if (arm == 1) return WEAR_LEFT_HAND;
    if (arm == 2) return WEAR_THIRD_HAND;
    if (arm == 3) return WEAR_FOURTH_HAND;
    return WEAR_RIGHT_HAND;
}


char * damtext_vs( int dam )
{
    if (dam <=   4) return  "bruises";
    else if (dam <=   6) return  "scratches";
    else if (dam <=   8) return  "grazes";
    else if (dam <=  12) return  "hits";
    else if (dam <=  16) return  "injures";
    else if (dam <=  20) return  "wounds";
    else if (dam <=  24) return  "mauls";
    else if (dam <=  28) return  "decimates";
    else if (dam <=  32) return  "devastates";
    else if (dam <=  36) return  "maims";
    else if (dam <=  42) return  "MUTILATES";
    else if (dam <=  48) return  "DISEMBOWELS";
    else if (dam <=  54) return  "DISMEMBERS";
    else if (dam <=  60) return  "MASSACRES";
    else if (dam <=  68) return  "MANGLES";
    else if (dam <=  76) return "> CRUSHES <";
    else if (dam <=  86) return  "+- DESECRATES -+";
    else if (dam <= 100) return  ">- DEMOLISHES -<";
    else if (dam <= 110) return  "*** DEVASTATES ***";
    else if (dam <= 130) return  "*** EVISCERATES ***";
    else if (dam <= 160) return  "=== OBLITERATES ===";
    else if (dam <= 200) return  ">>> ANNIHILATES <<<";
    else if (dam <= 240) return  "<<< ERADICATES >>>";
    else if (dam <= 280) return  ">-> LIQUEFIES <-<";
    else if (dam <= 320) return  "ooo PULVERISES ooo";
    else if (dam <= 360) return  "<*<*< NUKES >*>*>";
    else if (dam <= 400) return  "++++ PLASMERISES ++++";
    else if (dam <= 500) return  "***** TOTALLY DESTROYS *****";
    else 		 return  "--==>*>*> UTTERLY OBLITERATES <*<*<==--";
}

char * damtext_vp( int dam )
{
    if (dam <=   4) return  "bruising";
    else if (dam <=   6) return  "scratching";
    else if (dam <=   8) return  "grazing";
    else if (dam <=  12) return  "hitting";
    else if (dam <=  16) return  "injuring";
    else if (dam <=  20) return  "wounding";
    else if (dam <=  24) return  "mauling";
    else if (dam <=  28) return  "decimating";
    else if (dam <=  32) return  "devastating";
    else if (dam <=  36) return  "maiming";
    else if (dam <=  42) return  "MUTILATING";
    else if (dam <=  48) return  "DISEMBOWELING";
    else if (dam <=  54) return  "DISMEMBERING";
    else if (dam <=  60) return  "MASSACRING";
    else if (dam <=  68) return  "MANGLING";
    else if (dam <=  76) return "> CRUSHING <";
    else if (dam <=  86) return  "+- DESECRATING -+";
    else if (dam <= 100) return  ">- DEMOLISHING -<";
    else if (dam <= 110) return  "*** DEVASTATING ***";
    else if (dam <= 130) return  "*** EVISCERATING ***";
    else if (dam <= 160) return  "=== OBLITERATING ===";
    else if (dam <= 200) return  ">>> ANNIHILATING <<<";
    else if (dam <= 240) return  "<<< ERADICATING >>>";
    else if (dam <= 280) return  ">-> LIQUEFYING <-<";
    else if (dam <= 320) return  "ooo PULVERISING ooo";
    else if (dam <= 360) return  "<*<*< NUKING >*>*>";
    else if (dam <= 400) return  "++++ PLASMERISING ++++";
    else if (dam <= 500) return  "***** TOTALLY DESTROYING *****";
    else 		 return  "--==>*>*> UTTERLY OBLITERATING <*<*<==--";
}

char * damtext_v( int dam )
{
    if (dam <=   4) return  "bruise";
    else if (dam <=   6) return  "scratch";
    else if (dam <=   8) return  "graze";
    else if (dam <=  12) return  "hit";
    else if (dam <=  16) return  "injure";
    else if (dam <=  20) return  "wound";
    else if (dam <=  24) return  "maul";
    else if (dam <=  28) return  "decimate";
    else if (dam <=  32) return  "devastate";
    else if (dam <=  36) return  "maim";
    else if (dam <=  42) return  "MUTILATE";
    else if (dam <=  48) return  "DISEMBOWEL";
    else if (dam <=  54) return  "DISMEMBER";
    else if (dam <=  60) return  "MASSACRE";
    else if (dam <=  68) return  "MANGLE";
    else if (dam <=  76) return "> CRUSH <";
    else if (dam <=  86) return  "+- DESECRATE -+";
    else if (dam <= 100) return  ">- DEMOLISH -<";
    else if (dam <= 110) return  "*** DEVASTATE ***";
    else if (dam <= 130) return  "*** EVISCERATE ***";
    else if (dam <= 160) return  "=== OBLITERATE ===";
    else if (dam <= 200) return  ">>> ANNIHILATE <<<";
    else if (dam <= 240) return  "<<< ERADICATE >>>";
    else if (dam <= 280) return  ">-> LIQUEFY <-<";
    else if (dam <= 320) return  "ooo PULVERISE ooo";
    else if (dam <= 360) return  "<*<*< NUKE >*>*>";
    else if (dam <= 400) return  "++++ PLASMERISE ++++";
    else if (dam <= 500) return  "***** TOTALLY DESTROY *****";
    else 		 return  "--==>*>*> UTTERLY OBLITERATE <*<*<==--";
}

int get_punch_dam(CHAR_DATA *ch)
{
    int adder = 0;

    adder += dice(1, get_size(ch) * get_size(ch)) + GET_DAMROLL(ch) / 3;

    if (ch->power[DISC_LUPI_LEON] >= 1) adder *= 5 / 2;
    if (ch->power[DISC_LUPI_HAWK] >= 3) adder *= 5 / 2;

    return adder;
}

void do_kick(CHAR_DATA *ch, char *argument)
{
    int chance;
    CHAR_DATA *victim;
    int dam, dam_type, sn, dt, stmod, attack_modify, diceroll, didhit;

    if (lost_both_legs(ch))
    {
	stc("But you have no legs!\n\r", ch);
	return;
    }

    if (lost_one_leg(ch))
    {
	stc("You cannot kick with only one leg.\n\r", ch);
	return;
    }

    if ((victim = ch->fighting) == NULL)
    {
	stc("You can only kick while fighting.\n\r", ch);
	return;
    }

    chance = misc_body_combat_penalty( ch, 66 );

    if (dice(1, 100) >= chance)
    {
        stc("You are in no position to kick.\n\r", ch);
        return;
    }

    if (!will_attack( ch, ch->initiative[4], initiative_cost(ch, NULL)))
    {
        stc("You are in no position to kick.\n\r", ch);
        return;
    }

    if (ch->distance > attack_reach(ch, NULL))
    {
        stc("You are too far away to kick them.\n\r", ch);
        return;
    }

    WAIT_STATE(ch, 12);
    dam = 0;
    sn = gsn_kick;
    dt = TYPE_HIT + ch->dam_type;
    dam_type = DAM_BASH;
    ch->initiative[4] -= initiative_cost(ch, NULL);
    dam = get_punch_dam(ch);
    if (ch->cur_form == FRM_CRIMSON)
    {
        dam_type = DAM_NEGATIVE;
        dam = dam * 3 / 2;
    }
    else if (ch->cur_form == FRM_WHIRLWIND)
    {
        dam_type = DAM_BASH;
        dam = dam * 6 / 5;
    }
    else if (ch->cur_form == FRM_FLAME)
    {
        dam_type = DAM_FIRE;
        dam = dam * 3 / 2;
    }
    else if (ch->cur_form == FRM_LIGHT)
    {
        dam_type = DAM_LIGHTNING;
        dam = dam * 5 / 4;
    }

    if (get_skill(ch, gsn_enhanced_damage) > 0)
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch, gsn_enhanced_damage))
        {
	    check_improve(ch, gsn_enhanced_damage,TRUE,6);
	    dam += 2 * (dam * diceroll / 300);
	}
    }

    if (stances[ch->stance].score[victim->stance] >= 0)
	stmod = (stances[ch->stance].score[victim->stance] * 8 * ch->stance_skill[ch->stance])/100 + 100;
    else
    {
	stmod = ((stances[ch->stance].score[victim->stance]*ch->stance_skill[ch->stance])/100+20)*5;
	if (stmod < 10) stmod = 10;
    }
    if ((dice(1, 5000) < (170 - (ch->stance_skill[ch->stance]) + ch->mod_stat[STAT_INT])) && (ch->stance!= 0))
    {
	if (dice(1, 2) == 1)
	    improve_stance(ch, ch->stance);
    }
    dam = (dam * stmod) / 100;

    dam += GET_DAMROLL(ch) / 2;
 
    if ((didhit=did_hit(ch, victim, sn, dam_type)) > 0)
    {
        if (didhit == 19) dam *= 2;
        dam = cap_dam(ch, victim, dam, dam_type, TRUE);
	crash("called cap dam place 1");
        attack_modify = dice(1, 100);
        dam = randomize_damage( ch, dam, attack_modify );
        display_kick_damage(ch, victim, dam);
        if (deal_damage(ch, victim, dam)) return;
        if (dam > 200 && victim->distance < MAX_DIST)
        {
	    ch->distance += dam / 100;
	    if (ch->distance > MAX_DIST) ch->distance = MAX_DIST;
	    victim->distance = ch->distance;
	    stc("You are thrown backwards by the force of the blow.\n\r", victim);
	    act("$n is thrown back by the force of the blow.", victim, NULL, NULL, TO_ROOM);
        }
    }
    else
    {
        display_kick_miss(ch, victim);
    }
}
