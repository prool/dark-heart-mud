/******************************************************************
 * Core.c   -- (C) 1996, by Christopher Mellish                   *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hell.h"
#include "magic.h"
#include "recycle.h"


DECLARE_DO_FUN( do_group	);
DECLARE_DO_FUN( do_get	);
DECLARE_DO_FUN( do_sacrifice	);
DECLARE_DO_FUN( do_flee	);

void do_statarea(CHAR_DATA *ch, char *argument)
{
    AUTO_LOOK_DATA *ald;

    stc("AUTO MESSAGES INDOORS:\n\n\r", ch);
    for (ald = ch->in_room->area->auto_mesg_indoors; ald != NULL; ald = ald->next)
	stc(ald->text, ch);
    stc("\n\rAUTO MESSAGES OUTSIDE:\n\n\r", ch);
    for (ald = ch->in_room->area->auto_mesg_outside; ald != NULL; ald = ald->next)
	stc(ald->text, ch);
    stc("\n\rSo there.\n\r", ch);
 }

void do_bauble(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int type = 0;

    if (argument[0] == 0)
    {
	stc("Make what bauble?\n\r", ch);
	stc("'bauble types for' a list,\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "types"))
    {
	stc("You know how to make the following types of bauble:\n\r", ch);
	stc("    fire wall\n\r", ch);
	stc("    restoration\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "fire wall"))  type = 1;
    if (!str_prefix(argument, "restoration"))  type = 2;

    if (type == 0)
    {
	stc("You know of no such bauble type.\n\r", ch);
	return;
    }

    obj = create_object( get_obj_index( OBJ_VNUM_BAUBLE ), 0 );
    obj_to_char(obj, ch);
    obj->item_type = ITEM_BAUBLE;
    obj->value[0] = type;
    stc("You click you fingers and a bauble appears.\n\r", ch);
}

void smash_bauble( CHAR_DATA *ch, OBJ_DATA *obj )
{
    CHAR_DATA *rch, *rch_next;
    int dam;

    if (ch->in_room == NULL) return;

    act("$n drops a small bauble which shatters when it hits the ground,", ch, NULL, NULL, TO_ROOM);

    if (obj->value[0] == 2)
    {
	for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
	{
	    rch_next = rch->next_in_room;
	    restore_char( rch );
	}
    }
    else if (obj->value[0] == 1)
    {
	// fire wall bauble
	int door;

	act("releasing a massive explosion of flame.", ch, NULL, NULL, TO_ROOM);
	stc("When the bauble hits the ground, it shatters, releasing a devastating\n\rexplosion of flame, which burns all those around you.\n\r", ch);
	
	for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
	{
	    rch_next = rch->next_in_room;
	    if (rch == ch || rch->secondaryrace == 0)
	    {
		stc("You are not affected by the flames as they pass.\n\r", rch);
		continue;
	    }
	    dam = dice(10, 50);
	    stcf(rch, "The flames %s you as they pass.\n\r", damtext_v(dam));
	    deal_damage(ch, rch, dam);
	}
	act("As the flames reach the boundarys of the room, they stay, blocking the exits.", ch, NULL, NULL, TO_ROOM);
	stc("As the flames reach the boundarys of the room, they stay, blocking the exits.\n\r", ch);
	for (door = 0; door < 4; door++)
	{
	    if (ch->in_room->exit[door] != NULL)
		make_wall(ch->in_room, door, EX_FIRE_WALL);
	}
    }
    extract_obj(obj);
}

void do_screensplit( CHAR_DATA *ch, char *argument )
{
    if (argument[0] == 0)
    {
	stc("Screen split on or off?\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "on"))
    {
	ansi_split_screen( ch );
	return;
    }
    else
    {
	ansi_reset( ch );
	ansi_cls( ch );
    }
}


char * banner_name(char *bname)
{
    int i,j = 0;
    char *buf;
    buf = alloc_mem((strlen(bname) * 2) + 1);

    for (i=0 ; i<strlen(bname) ; i++)
    {
	buf[j++] = UPPER(bname[i]);
	buf[j++] = ' ';
    }

    buf[j] = '\0';
    return buf;
}

void do_dragonpromote(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

	if (str_cmp(GET_PC_NAME(ch), "Mitzrah"))
	{
	    stc("Only the leader of the dragons may use this ability.\n\r",ch);
	    return;
	}

    if (argument[0] == 0)
    {
	stc("dragon promote who?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL)
    {
	stc("They are not playing.\n\r",ch);
	return;
    }

    if (victim->secondaryrace != RACE_DRAGON)
    {
	stc("They are not a dragon.\n\r",ch);
	return;
    }

    SET_TIMER(victim, TIMER_DRAGON_GROW, 1);
    stc("Promotion will occur next tick.\n\r",ch);
}




void do_makebloodpool(CHAR_DATA *ch, char *argument)
{
        OBJ_DATA *obj;                                                          
        int onum = 0;                                                           
	int dragtype;

	if (str_cmp(GET_PC_NAME(ch), "Mitzrah"))
	{
	    stc("Only the leader of the dragons may use this ability.\n\r",ch);
	    return;
	}

	if (!str_prefix(argument, "red"		)) dragtype = DRAGON_RED;
   else if (!str_prefix(argument, "blue"	)) dragtype = DRAGON_BLUE;
   else if (!str_prefix(argument, "white"	)) dragtype = DRAGON_WHITE;
   else if (!str_prefix(argument, "black"	)) dragtype = DRAGON_BLACK;
   else if (!str_prefix(argument, "shadow"	)) dragtype = DRAGON_SHADOW;
   else if (!str_prefix(argument, "silver"	)) dragtype = DRAGON_SILVER;
   else if (!str_prefix(argument, "gold"	)) dragtype = DRAGON_GOLD;
   else if (!str_prefix(argument, "green"	)) dragtype = DRAGON_GREEN;
   else
   {
	stc("Syntax:  makebloodpool <dragon colour>\n\r", ch);
	return;
   }

        if (dragtype == DRAGON_RED) onum = 102;                         
        else if (dragtype == DRAGON_BLUE) onum = 104;                   
        else if (dragtype == DRAGON_WHITE) onum = 100;                  
        else if (dragtype == DRAGON_BLACK) onum = 101;                  
        else if (dragtype == DRAGON_SHADOW) onum = 111;             
        else if (dragtype == DRAGON_SILVER) onum = 105;                 
        else if (dragtype == DRAGON_GOLD) onum = 106;                   
        else if (dragtype == DRAGON_GREEN) onum = 103;              
                                                                                
        if (onum != 0)                                                      
        {                                                                    
            obj = create_object(get_obj_index(onum), 0);     
            obj_to_room(obj, ch->in_room, ch->altitude);      
            obj->value[0] = dragtype;
            obj->timer = 2;                                                 
            act("A pool of blood flows all over the floor.", ch, NULL, NULL, TO_ALL);
        }                                                                       
}

void banner_buffer(char *bname, char *buf)
{
    int i,j = 0;
    for (i=0 ; i<strlen(bname) ; i++)
    {
	if (bname[i] == '#')
	{
	    i += 2;
	}
	buf[j++] = UPPER(bname[i]);
	buf[j++] = ' ';
    }

    buf[j] = '\0';
}

void do_timer(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (count_down > -1)
	{
	    sprintf(buf, "%d seconds to %s\n\r", count_down,count_down_to);
	    stc(buf, ch);
	    return;
	}
	stc("There is currently no count down timer.\n\r",ch);
	stc("Count down to ", ch);
	stc(count_down_to, ch);
	stc("\n\r",ch);
	return;
    }

    if (is_number(argument))
    {
	int i;

	i = atoi(argument);
	sprintf(buf, "Timer started for %d seconds.\n\r",i);
	stc(buf, ch);
	count_down = i;
	second_update();
	return;
    }

    WRITE_STR(count_down_to, argument);
	stc("Count down to ", ch);
	stc(count_down_to, ch);
	stc("\n\r",ch);
}

void wrap_to_char(char *argument, CHAR_DATA *ch)  
{
           char tempbuf[MAX_STRING_LENGTH];
           char wordbuf[MAX_INPUT_LENGTH];
            int width = 0;

            tempbuf[0] = '\0';
            for ( ; ; )
            {
                /* copy next word into wordbuf */  
                argument = split_misc(argument, wordbuf, ' ');
                if (wordbuf[0] == '\0')
                    break;
                if (strlen(wordbuf) + width > 79)
                {
                    strcat(tempbuf, "\r\n");
                    width = 0;
                }
                if (width > 0)
                {
                    strcat(tempbuf, " ");
                    width++;
                }
                width += strlen(wordbuf);
                strcat(tempbuf, wordbuf);
            }
            strcat(tempbuf, "\n\r");
	send_to_char(tempbuf, ch);             
}

void show_vision(CHAR_DATA *ch)
{
    send_to_char("Command lost in crash.\n\r",ch);
}

void do_history(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    int i, j, len, pos;

    banner2_to_char("COMMAND HISTORY", ch);

    for (i = 1; i < 11; i++)
    {
	j = ch->desc->inlasti - i + 1;
	if (j < 0) j += 10;
	if (j > 10) j -= 1;

	len = strlen(ch->desc->inlast[j]);

	if (len > 40)
	{
	    strcpy(buf2, ch->desc->inlast[j]);
	    buf2[20] = '\0';
	    pos = len - 20;
	    sprintf(buf, "%d. %-20s ... %-20s", (i < 10) ? i : 0,
		buf2, &(buf2[pos]));
	}
	else sprintf(buf, "%d. %-40s     ", (i < 10) ? i : 0, ch->desc->inlast[j]);
	cent_to_char(buf, ch);
    }

    divide6_to_char(ch);
}

/* Janders induct command, replacing the lost one */
void do_induct(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    /* Temp fix... maybe have clan heads??? not sure yet.  */
    
    if (ch->clan == CLAN_NONE)
    {
	stc("You aren't in a clan.\n\r", ch);
	return;
    }

    if (!is_clan_leader(ch))
    {
	stc("Only a clan leader may induct people.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Syntax : induct <character>\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	stc("They aren't here.\n\r", ch);
	return;
    }

    if (ch->secondaryrace != victim->secondaryrace)
    {
	stc("They must be of the same secondary race as you.\n\r", ch);
	return;
    }

/*    if (ch->secondaryrace == RACE_LUPINE
	&& ch->dragtype != victim->dragtype)
    {
	stc("They are not of the same phenotype as you.\n\r", ch);
	return;
    } */

    if (ch->tertiaryrace == TERTIARY_DROW
	|| victim->tertiaryrace == TERTIARY_DROW)
    {
	stc("Drow cannot change clans or induct.\n\r", ch);
	return;
    } 

    if (ch->clan == CLAN_WARL_CRIMSON
	&& victim->primary != REALM_FLAME)
    {
	stc("Only flame mages may join the crimson flame.\n\r", ch);
	return;
    }

    if (ch->clan == 0 || ch->clan == CLAN_DAEM_OUTCAST
	|| ch->clan == CLAN_LUPI_ROGUE
	|| ch->clan == CLAN_VAMP_ANARCH)
    {
	stc("You aren't in a clan.\n\r", ch);
	return;
    }

    if (victim->clan != 0 && victim->clan != CLAN_DAEM_OUTCAST
	&& victim->clan != CLAN_LUPI_ROGUE
	&& victim->clan != CLAN_VAMP_ANARCH)
    {
	stc("They are already in a clan.\n\r", ch);
	return;
    }

    act("$N joins your clan.", ch, NULL, victim, TO_CHAR);
    act("You join $n's clan.", ch, NULL, victim, TO_VICT);
    WAIT_STATE(ch, 8);
    WAIT_STATE(victim, 8);
    victim->clan = ch->clan;
    set_clan_disciplines(victim);

    if (ch->clan == CLAN_VAMP_MANTICORE)
    {	
	stc("You gain the learning of the manticore.\n\r", victim);
    }

    if (ch->clan == CLAN_LUPI_AETHIR)
    {
	stc("You gain the learning of the hawk.\n\r", victim);
	victim->power[DISC_LUPI_HAWK] = 2;
    }
}

void do_fightprompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];                                              
                                                                             
   if ( argument[0] == '\0' )                                                
   {                                                                         
	stcf(ch, "Your current fight prompt is:\n\r%s\n\r", ch->fightprompt);
       	return;                                                                
   }      
                                                                    
   if( !strcmp( argument, "all" ) )           
      strcpy( buf, "<%o %hhp %vmv %ppsp [%d]> ");                                    
   else                                                                      
   {                                                                         
      if ( strlen(argument) > 70 )                                           
         argument[70] = '\0';                                                 
      strcpy( buf, argument ); 
      smash_tilde( buf );                                           
      if (str_suffix("%c",buf))                                                                                             
	strcat(buf, " ");
   }                                                                         
                                                                             
   free_string( ch->fightprompt );                                                
   ch->fightprompt = str_dup( buf );                                              
   sprintf(buf,"Fight Prompt set to %s\n\r",ch->fightprompt );                           
   send_to_char(buf,ch);       
   return;                                                          
}

void do_primary(CHAR_DATA *ch, char *argument)
{
    int realm;

    if (IS_NPC(ch))
    {
	send_to_char("Mobs cannot have a primary realm.\n\r", ch);
	return;
    }

    if (ch->secondaryrace == 0)
    {
	send_to_char("You cannot cast spells yet.\n\r", ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Syntax : primary [#1flame #4water #6air #2earth #5entropy#n]\n\r", ch); 
	return;
    }

    if (ch->primary != 0 && ch->level != 120 )
    {
	send_to_char("You already have a primary realm.\n\r",ch);
	return;
    }

    if (!str_cmp(argument, "flame")) realm = 1;
    else if (!str_cmp(argument, "water")) realm = 2;
    else if (!str_cmp(argument, "air")) realm = 3;
    else if (!str_cmp(argument, "earth")) realm = 4;
    else if (!str_cmp(argument, "entropy")) realm = 5;
    else if (!str_cmp(argument, "life")) realm = 6;
    else
    {
	do_primary(ch, "");
	return;
    }

    if (realm == 5 && IS_GOOD(ch) )
    {
	send_to_char("Only evil players may set entropy as a primary circle.\n\r", ch);
	return;
    }

    if (realm == 1 && ch->fire < 3)
    {
	send_to_char("You need more experience at flame spells.\n\r", ch);
	return;
    }
    if (realm == 2 && ch->water < 3)
    {
	send_to_char("You need more experience at water spells.\n\r", ch);
	return;
    }
    if (realm == 3 && ch->air < 3)
    {
	send_to_char("You need more experience at air spells.\n\r", ch);
	return;
    }
    if (realm == 4 && ch->earth < 3)
    {
	send_to_char("You need more experience at earth spells.\n\r", ch);
	return;
    }
    if (realm == 5 && ch->entropy < 3)
    {
	send_to_char("You need more experience at entropy spells.\n\r", ch);
	return;
    }

    ch->primary = realm;
    send_to_char("Your primary circle has been set.\n\r", ch);

}

void do_vision(CHAR_DATA *ch, char *argument)
{
    send_to_char("Command lost in crash.\n\r",ch);
}

void do_language(CHAR_DATA *ch, char *argument)
{
    if (argument[0] == '\0')
    {
	send_to_char("You can speak the following languages:\n\r", ch);
	send_to_char("common    ",ch);
	if (can_speak(ch, LANG_ELVISH)) 
	    send_to_char("elvish    ",ch);
	if (can_speak(ch, LANG_GNOMISH))
	    send_to_char("gnome     ",ch);
	if (can_speak(ch, LANG_DEMONIC))
	    send_to_char("demon",ch);
	send_to_char("\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "common"))
    {
	ch->curlang = LANG_COMMON;
	send_to_char("You will now talk in Common.\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "elvish"))
    {
	if (!can_speak(ch, LANG_ELVISH))
	{
	    send_to_char("You do not know this language.\n\r",ch);
	    return;
	}

	ch->curlang = LANG_ELVISH;
	send_to_char("You will now talk in Elvish.\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "gnome"))
    {
	if (!can_speak(ch, LANG_GNOMISH))
	{
	    send_to_char("You do not know this language.\n\r",ch);
	    return;
	}

	ch->curlang = LANG_GNOMISH;
	send_to_char("You will now talk in Gnome.\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "demon"))
    {
	if (!can_speak(ch, LANG_DEMONIC))
	{
	    send_to_char("You do not know this language.\n\r",ch);
	    return;
	}

	ch->curlang = LANG_DEMONIC;
	send_to_char("You will now talk in the tongue of the Demons.\n\r",ch);
	return;
    }

    send_to_char("You do not know this language.\n\r",ch);
}

void do_dragon_devour(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;

    if (argument[0] == '\0')
    {
	send_to_char("Devour whom?\n\r",ch);
	return;
    }

    if ((vch = get_char_room(ch, argument))==NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch, vch)) return;

    if (vch > 0)
    {
	stc("You may only devour a mortally wounded enemy.\n\r", ch);
	return;
    }

    if (get_size(ch) < get_size(vch))
    {
	send_to_char("They are too large for you to devour.\n\r",ch);
	return;
    }
    if (vch->position > POS_SLEEPING)
    {
        if ((vch->hit > 0 && dice(1, get_curr_stat(vch, STAT_DEX))*2 > (get_curr_stat(ch, STAT_DEX) / 4)) || vch->level == 120)
        {
	    send_to_char("You try to bite them in half, but they dodge your attack.\r\n",ch);
	    act("$n tries to bite $N in half, but $E rolls out of the way",ch, NULL,vch,TO_NOTVICT);
	    act("$n tries to bite you in half, but you roll out of the way.",ch, NULL,vch,TO_VICT);
	    WAIT_STATE(ch, 12);
	    return;
        }
    }

    WAIT_STATE(ch, 12);
    act("You reach down and pick $N up in your mouth, then bite hard and swallow.",ch,NULL,vch,TO_CHAR);
    act("$n picks $N up in $s mouth and swallows $M whole.",ch,NULL,vch,TO_NOTVICT);
    send_to_char("You feel every bone in your body shatter as something bites you in half.\n\r",vch);
    destroy_char(vch, vch, FALSE);
    if (player_kill(ch, vch)) player_kill_update(ch, vch, FALSE);
    sprintf(buf, "--> %s has been devoured by %s.", GET_NAME(vch), GET_NAME(ch));
    informational(buf, vch->secondaryrace, 0);
    informational(buf, ch->secondaryrace, 0);
}

void do_email(CHAR_DATA *ch, char *argument)
{
    bool fnd = FALSE;
    int i;

    if (IS_NPC(ch)) return;

    if (ch->email_set != 0)
    {
	stc("You cannot change your email address once you have registerred.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("What do you want to set your email address to?\n\r",ch);
	return;
    }

    for (i = 0; argument[i] != '\0'; i++)
    {
	if (argument[i] == ';')
	{
	    stc("You cheating swine.  No semi-colons.\n\r", ch);
	    return;
	}
	if (argument[i] == '@') { fnd = TRUE; continue; }
    }

    if (!fnd)
    {
	stc("Invalid email address.\n\r",ch);
	return;
    }

    if (ch->pcdata->email != NULL)
	free_string(ch->pcdata->email);

    ch->pcdata->email = str_dup(argument);

    send_to_char("Your email address is now set to: ",ch);
    send_to_char(ch->pcdata->email,ch);
    send_to_char("\n\r",ch);
}

void do_homepage(CHAR_DATA *ch, char *argument)
{
    int i;

    if (IS_NPC(ch)) return;

    if (argument[0] == '\0')
    {
	send_to_char("What do you want to set your home page address to?\n\r",ch);
	return;
    }

    for (i = 0; argument[i] != '\0'; i++)
    {
	if (argument[i] == ';')
	{
	    stc("You cheating swine.  No semi-colons.\n\r", ch);
	    return;
	}
    }

    if (ch->pcdata->homepage != NULL)
	free_string(ch->pcdata->homepage);

    ch->pcdata->homepage = str_dup(argument);

    send_to_char("Your home page address is now set to: ",ch);
    send_to_char(ch->pcdata->homepage,ch);
    send_to_char("\n\r",ch);
}

void do_realname(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (argument[0] == '\0')
    {
	send_to_char("Well what is your real name?\n\r",ch);
	return;
    }

    smash_tilde(argument);

    if (ch->pcdata->real_name != NULL)
	free_string(ch->pcdata->real_name);

    ch->pcdata->real_name = str_dup(argument);

    send_to_char("Your real name is now ",ch);
    send_to_char(ch->pcdata->real_name,ch);
    send_to_char(".\n\r",ch);
}

void do_plan(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (argument[0] == '\0')
    {
	send_to_char("What do you want to set your plan to?\n\r",ch);
	return;
    }

    smash_tilde(argument);

    if (ch->pcdata->plan != NULL)
	free_string(ch->pcdata->plan);

    ch->pcdata->plan = str_dup(argument);

    send_to_char("Your plan is now set to: ",ch);
    send_to_char(ch->pcdata->plan,ch);
    send_to_char("\n\r",ch);
}

void do_propose(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;

    if (argument[0] == '\0')
    {
	send_to_char("Propose to who?\n\r",ch);
	return;
    }

    if (ch->married_to != NULL && ch->marriage_status > MARRY_PROPOSING)
    {
	switch ( ch->marriage_status )
	{
	    case MARRY_ENGAGED : 
		send_to_char("You are already engaged.\n\r",ch);
		break;

	    case MARRY_MARRIED :
		send_to_char("You are already married.\n\r",ch);
		break;
	}

	return;
    }

    if ((vch = get_char_room(ch, argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    act("You get down on bended knee and lovingly propose to $N.", ch, NULL, vch, TO_CHAR);
    act("$n gets down on bended knee and lovingly proposes to you.",ch,NULL, vch, TO_VICT);
    act("$n gets down on bended knee and lovingly proposes to $N.",ch,NULL, vch, TO_NOTVICT);

    if (ch->married_to != NULL)
	free_string(ch->married_to);
    ch->married_to = str_dup(GET_NAME(vch));
    ch->marriage_status = MARRY_PROPOSING;
}

void do_accept(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;

    if (ch->married_to != NULL && ch->marriage_status > MARRY_PROPOSING)
    {
	switch ( ch->marriage_status )
	{
	    case MARRY_ENGAGED : 
		send_to_char("You are already engaged.\n\r",ch);
		break;

	    case MARRY_MARRIED :
		send_to_char("You are already married.\n\r",ch);
		break;
	}

	return;
    }

    if (argument[0]=='\0')
    {
	send_to_char("Accept whose proposal?\n\r",ch);
	return;
    }

    if ((vch = get_char_room(ch, argument))==NULL)
    {
	send_to_char("They are not here.\n\r", ch);
	return;
    }

    if (vch->marriage_status != MARRY_PROPOSING
	|| vch->married_to == NULL
	|| str_cmp(vch->married_to, ch->name))
    {
	send_to_char("They haven't proposed to you.\n\r", ch);
	return;
    }

    act("$n accepts your proposal.  You are now engaged.",ch,NULL,vch,TO_VICT);
    act("You accept $N's proposal.  You are now engaged.",ch,NULL,vch,TO_CHAR);

    if (ch->married_to != NULL)
	free_string(ch->married_to);
    ch->married_to = str_dup(vch->name);
    ch->marriage_status = MARRY_ENGAGED;
    vch->marriage_status = MARRY_ENGAGED;
}

void do_shapeshift(CHAR_DATA *ch, char *argument)
{
    
}

/*void do_man( CHAR_DATA *ch, char *argument )
{
    ch->desc->connected = CON_MANUAL;
    ch->pcdata->curman = manual_first;
    do_manual(ch, "");
}

void do_manual( CHAR_DATA *ch, char *argument )
{
    MANUAL_DATA *pMan, *pManual;
    BUFFER *output;
    bool found = FALSE, nl;
    int level,lnk;

    output = new_buf();
    
    if (!str_cmp(argument, "home"))
	ch->pcdata->curman = manual_first;
    
    if ( argument[0] == '\0' || !(is_number(argument) && strlen(argument) == 1))
    {
	pManual = ch->pcdata->curman;
	found = TRUE;
    }
    else
    {
	lnk = atoi(argument[0]);
	if (ch->pcdata->curman->link[lnk][0]=='\0')
	{
	    found = FALSE;
	}
	else
	{
	    for ( pMan = manual_first; (pMan != NULL || found); pMan = pMan->next )
	    {
		level = (pMan->level < 0) ? -1 * pMan->level - 1 : pMan->level;

		if (level > get_trust( ch ) )
		    continue;

		if ( is_name( ch->pcdata->curman->link[lnk], pMan->keyword ) )
		{
		    found = TRUE;
		    pManual = pMan;
		}

	    }
	}

    if (!found)
	send_to_char( "That link does not exist.\n\r", ch );
    else
    {
	if (pManual->level >= 0)
	{
	    add_buf(output, pManual->keyword);
	    add_buf(output, "\n\r");
		if ( pManual->text[0] == '.' )
		    add_buf(output,pManual->text+1);
		else
		add_buf(output,pManual->text);
	    nl = FALSE;
	    for (i = 0; i < 10; i++)    
	    {
		if (pManual->linkname[i][0] !="\0")
		{
		    sprintf(buf, "%d:  %-32s    ", i, pManual->linkname[i]);
		    add_buf(output, buf);
		    if (nl)
		    {
			add_buf(output,"\n\r");
			nl = FALSE;
		    }
		    else
		    {
			nl = TRUE;
		    }
		}
	    }
	    add_buf(output,"   [RETURN] - Redisplay Page    'play' - exit manual   'home' - Welcome Page\r\n");
	page_to_char(buf_string(output),ch);
    }
    free_buf(output);
}*/

void do_entrance(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    
    if (ch->blood < 5)
    {
	send_to_char("You do not have enough blood to burn.\n\r", ch);
	return;
    }
    
    if (argument[0]=='\0')
    {
	send_to_char("Who do you wish to try to charm?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch, argument))==NULL)
    {
	send_to_char("They are not here.\n\r",ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (victim->master == ch)
    {
	send_to_char("They are already under your control.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 12);

    if (victim->secondaryrace == RACE_VAMPIRE
	&& victim->power[DISC_VAMP_DOMI] > ch->power[DISC_VAMP_DOMI] )
    {
	act("As you stare into $N's eyes, you feel your willpower slipping away.", ch, NULL, victim, TO_CHAR);
	send_to_char("Perhaps that was not a good idea.\n\r", ch);
	act("$n stares into your eyes, attempting to charm you, but $e fails.", ch, NULL, victim, TO_VICT);
	act("$n stares deeply into $N's eyes.", ch, NULL, victim, TO_NOTVICT);
	return;
    }

/*	&& victim->pcdata->conditioned_by != ch->pcdata->switchname))*/

    if ( victim->level > ch->level
	|| (ch->level - victim->level) < dice(1, 20) )
    {
	send_to_char("You gaze deeply into their eyes, but they shrug it off and back away.\n\r",ch);
	act("$n gazes deeply into $N's eyes, and $N backs away.",ch,NULL,victim,TO_NOTVICT);
	act("$n gazes into your eyes.  You feel a tingling sensation, but\n\ryou shrug it off.",ch,NULL,victim,TO_VICT);
	return;
    }
    
    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = gsn_charm_person;
    af.level     = ch->level;
    af.duration  = ch->generation;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    act( "$n stares into your eyes.  You feel compelled to obey $m.", ch, NULL, victim, TO_VICT );
    act( "$n gazes at $N, and they stare lovingly back.",ch,NULL,victim,TO_ROOM);
    if ( ch != victim )
	act("$N is now under your power.",ch,NULL,victim,TO_CHAR);
    ch->blood -= 5;
}

void do_embrace(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (ch->sucking != NULL)
    {
	if (dice(1, 20) > ch->generation + 5)
 	{
	    stc("You cannot break the embrace.\n\r", ch);
	    WAIT_STATE(ch, 8);
	    return;
 	}
	send_to_char("You retract your fangs and back away.\n\r", ch);
	act("$n retracts $s fangs and backs away.", ch, NULL, NULL, TO_ROOM);
	REMOVE_BIT(ch->flag2, AFF_BITE);
	REMOVE_BIT(ch->sucking->flag4, AFF_BEING_SUCKED);
	ch->sucking = NULL;
	WAIT_STATE(ch, 4);
	return;
    }

    if (argument[0]=='\0')
    {
	send_to_char("Who do you wish to embrace?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch, argument))==NULL)
    {
	send_to_char("You can't see them here.\n\r",ch);
	return;
    }

    if (is_safe(ch, victim)) return;

    if (victim == ch)
    {
	stc("You can't embrace yourself!\n\r",ch);
	return;
    }

    if (!IS_NEWFLAG(ch, AFF_BITE) )
    {
	stc("Your fangs slide out of your gums.\n\r", ch);
	act("Sharp fangs slide out of $n's gums.", ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->flag2, AFF_BITE);
    }

    if (victim->master != ch
	&& victim->position > POS_SLEEPING)
    {
	send_to_char("You try to embrace them, but they back away.\n\r",ch);
	act("$n tries to embrace you, but you back away.",ch,NULL,victim,TO_VICT);
	act("$n tries to embrace $N, but $E backs away.",ch,NULL,victim,TO_ROOM);
	return;
    }

    if (get_size(victim) > get_size(ch))
    {
	send_to_char("They are too large to embrace.\n\r", ch);
	return;
    }

    act("You sink your fangs deep into $N's neck and begin to drink $S blood.", ch, NULL, victim, TO_CHAR);
    act("$n sinks $s fangs into your neck and begins to drink.", ch, NULL, victim, TO_VICT);
    act("$n sinks $s fangs into $N's neck and begins to drink $S blood.", ch, NULL, victim, TO_NOTVICT);
    ch->sucking = victim;
    SET_BIT(ch->sucking->flag4, AFF_BEING_SUCKED);
}

void do_room_display(ROOM_INDEX_DATA *room, char *argument)
{
    CHAR_DATA *ch;

    for (ch = room->people; ch != NULL; ch = ch->next_in_room)
	send_to_char(argument, ch);
}

void do_room_say(ROOM_INDEX_DATA *room, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "A voice booms '%s'", argument);
    do_room_display(room, buf);
}

void obj_say(OBJ_DATA *obj, char *argument, bool muffled, CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (ch != NULL)
	sprintf(buf2, " carried by %s", GET_NAME(ch));

    if (muffled)
	sprintf(buf, "You think you hear a muffled voice say '%s'", argument);
    else
	sprintf(buf, "%s%s says '%s'", obj->short_descr,
		(ch != NULL) ? buf2 : "",
		argument);
    
    if (obj->in_room != NULL)
    {
	do_room_display(obj->in_room, buf);
	return;
    }

    if (ch != NULL)
    {
	act(buf, obj->carried_by, NULL, NULL, TO_ROOM);
	if (!muffled) sprintf(buf,"%s that you are carrying says '%s'",obj->short_descr, argument);
	send_to_char(buf, ch);
	send_to_char("\n\r",ch);
	return;
    }
    
    if (obj->carried_by != NULL)
    {
	obj_say(obj, argument, FALSE, obj->carried_by);
	return;
    }

    if (obj->in_obj != NULL)
    {
	obj_say(obj->in_obj, argument, TRUE, NULL);
	return;
    }

    if (obj->on != NULL && obj->on->in_room != NULL)
    {
	sprintf(buf, "%s that is on %s says '%s'", obj->short_descr, obj->on->short_descr, argument);
	do_room_display(obj->on->in_room, buf);
	return;
    }
}

void do_langtest( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    conv_language(argument, buf, 1, ch);
    send_to_char(buf,ch);
    send_to_char("\n\r",ch);
    conv_language(argument, buf, 2, ch);
    send_to_char(buf,ch);
    send_to_char("\n\r",ch);
    conv_language(argument, buf, 3, ch);
    send_to_char(buf,ch);
    send_to_char("\n\r",ch);
}

void conv_language( char *argument, char *buf, int lang, CHAR_DATA *ch)
{
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *  old;
	char *  new;
    };

    static const struct syl_type lang_syl_table[NUM_LANGUAGES][100] =
    {
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
	},

	{
	{ " ",          " "             },
	{ "ar",         "abra"          },
	{ "au",         "kia"          },
	{ "bless",      "auva"          },
	{ "blind",      "nia"           },
	{ "bur",        "nae"          },
	{ "cu",         "laria"           },
	{ "de",         "soa"          },
	{ "en",         "na"          },
	{ "light",      "iloia"          },
	{ "lo",         "pria"           },
	{ "mor",        "vor"           },
	{ "move",       "lyi"          },
	{ "ness",       "maria"          },
	{ "ing",        "naia"          },
	{ "per",        "bro"         },
	{ "ra",         "poi"            },
	{ "war",        "marta"            },
	{ "fight",      "carna"          },
	{ "hel",        "pros"          },
	{ "tect",       "sas"        },
	{ "tri",        "prest"          },
	{ "ven",        "lao"          },
	{ "a", "e" }, { "b", "c" }, { "c", "i" }, { "d", "p" },
	{ "e", "o" }, { "f", "a" }, { "g", "h" }, { "h", "e" },
	{ "i", "a" }, { "j", "f" }, { "k", "i" }, { "l", "r" },
	{ "m", "t" }, { "n", "u" }, { "o", "u" }, { "p", "d" },
	{ "q", "b" }, { "r", "n" }, { "s", "a" }, { "t", "m" },
	{ "u", "o" }, { "v", "s" }, { "w", "s" }, { "x", "l" },
	{ "y", "u" }, { "z", "i" },
	{ "", "" }
	},

	{
	{ "stop",	"stop"		},
	{ "go",		"go"		},
	{ "north",	"north"		},
	{ "south",	"south"		},
	{ "east",	"east"		},
	{ "west",	"west"		},
	{ "sad",	"sad"		},
	{ "friend",	"friend"	},
	{ "dragon",	"dragon"	},
	{ "mage",	"mage"		},
	{ "warlock",	"warlock"	},
	{ "werewolf",	"werewolf"	},
	{ "hero",	"hero"		},
	{ "warrior",	"warrior"	},
	{ "vampire",	"vampire"	},
	{ "new",	"new"		},
	{ "old",	"old"		},
	{ "clan",	"clan"		},
	{ "eladrin",	"eladrin"	},
	{ "human",	"human"		},
	{ "elf",	"elf"		},
	{ "rest",	"rest"		},
	{ "wake",	"wake"		},
	{ "make",	"make"		},
	{ "bastard",	"bastard"	},
	{ "sleep",	"sleep"		},
	{ "me",		"me"		},
	{ "my",		"my"		},
	{ "you",	"you"		},
	{ "hello",	"hello"		},
	{ "hi",		"hello"		},
	{ "said",	"said"		},
	{ "say",	"say"		},
	{ "talk",	"talk"		},
	{ "food",	"food"		},
	{ "drink",	"drink"		},
	{ "water",	"water"		},
	{ "fight",	"fight"		},
	{ "hurt",	"hurt"		},
	{ "good",	"good"		},
	{ "bad",	"bad"		},
	{ "evil",	"evil"		},
	{ "nasty",	"nasty"		},
	{ "war",	"fun"		},
	{ "battle",	"battle"	},
	{ "magic",	"magic"		},
	{ "shhh",	"shhh"		},
	{ "quiet",	"quiet"		},
	{ "loud",	"loud"		},
	{ "kill",	"kill"		},
	{ "surprise",	"surprise"	},
	{ " ",		" "		},
	{ ".",		"."		},
	{ "!",		"!"		},
	{ ",",		","		},
	{ "",		""		},
	},

        {
	{ " ",          " "             },
	{ "ar",         "rib"          },
	{ "au",         "ska"          },
	{ "bless",      "gree"          },
	{ "blind",      "poon"           },
	{ "bur",        "smah"          },
	{ "cu",         "hoi"           },
	{ "de",         "glur"          },
	{ "en",         "kro"          },
	{ "light",      "maba"          },
	{ "lo",         "eeen"           },
	{ "mor",        "eur"           },
	{ "move",       "fah"          },
	{ "ness",       "pir"          },
	{ "ing",        "oik"          },
	{ "per",        "oin"         },
	{ "ra",         "tro"            },
	{ "war",        "blag"            },
	{ "fight",      "smeen"          },
	{ "hel",        "firg"          },
	{ "tect",       "reeb"        },
	{ "tri",        "skaah"          },
	{ "ven",        "frim"          },
	{ "a", "e" }, { "b", "c" }, { "c", "i" }, { "d", "p" },
	{ "e", "o" }, { "f", "a" }, { "g", "h" }, { "h", "e" },
	{ "i", "a" }, { "j", "f" }, { "k", "i" }, { "l", "r" },
	{ "m", "t" }, { "n", "u" }, { "o", "u" }, { "p", "d" },
	{ "q", "b" }, { "r", "n" }, { "s", "a" }, { "t", "m" },
	{ "u", "o" }, { "v", "s" }, { "w", "s" }, { "x", "l" },
	{ "y", "u" }, { "z", "i" },
	{ "", "" }
        },

	{
	{ " ",          " "             },
	{ "ar",         "rib"          },
	{ "au",         "ska"          },
	{ "bless",      "gree"          },
	{ "blind",      "poon"           },
	{ "bur",        "smah"          },
	{ "cu",         "hoi"           },
	{ "de",         "glur"          },
	{ "en",         "kro"          },
	{ "light",      "maba"          },
	{ "lo",         "eeen"           },
	{ "mor",        "eur"           },
	{ "move",       "fah"          },
	{ "ness",       "pir"          },
	{ "ing",        "oik"          },
	{ "per",        "oin"         },
	{ "ra",         "tro"            },
	{ "war",        "blag"            },
	{ "fight",      "smeen"          },
	{ "hel",        "firg"          },
	{ "tect",       "reeb"        },
	{ "tri",        "skaah"          },
	{ "ven",        "frim"          },
	{ "a", "e" }, { "b", "c" }, { "c", "i" }, { "d", "p" },
	{ "e", "o" }, { "f", "a" }, { "g", "h" }, { "h", "e" },
	{ "i", "a" }, { "j", "f" }, { "k", "i" }, { "l", "r" },
	{ "m", "t" }, { "n", "u" }, { "o", "u" }, { "p", "d" },
	{ "q", "b" }, { "r", "n" }, { "s", "a" }, { "t", "m" },
	{ "u", "o" }, { "v", "s" }, { "w", "s" }, { "x", "l" },
	{ "y", "u" }, { "z", "i" },
	{ "", "" }
	},

        {
        { " me ",        " me, me " },
        { " not ",     " " },
        { " ",         " " },
        { "ss",        "sssshh" },
        { "hate",      "love" },
        { "despise",   "fancy" },
        { "don't",     "" },
        { "pint",      "tit" },
        { "minger",    "babe"   },
        { "troll",     "fit bird" },
        { "alright",   "ace" },
        { "small",     "huge" },
        { "okay",      "okay, okay, okay," },
        { "",          "" },
        },

	{
	{ " ",          " "             },
	{ "ar",         "jan"          },
	{ "au",         "jeh"          },
	{ "bl",         "wet"          },
	{ "ba",         "qu"           },
	{ "ce",         "kae"          },
	{ "com",        "ilz"           },
	{ "de",         "que"          },
	{ "do",         "jeh"          },
	{ "ki",         "paz"          },
	{ "lan",        "fa"           },
	{ "ll",         "zeck"           },
	{ "move",       "bre"          },
	{ "ness",       "lon"          },
	{ "ing",        "sh"          },
	{ "per",        "lok"         },
	{ "ra",         "ka"            },
	{ "war",        "wert"            },
	{ "fight",      "karka"          },
	{ "hel",        "xar"          },
	{ "tect",       "tak"        },
	{ "tri",        "noz"          },
	{ "ss",         "zz"          },
	{ "a", "u" }, { "b", "d" }, { "c", "p" }, { "d", "q" },
	{ "e", "a" }, { "f", "w" }, { "g", "h" }, { "h", "c" },
	{ "i", "o" }, { "j", "s" }, { "k", "n" }, { "l", "k" },
	{ "m", "t" }, { "n", "t" }, { "o", "i" }, { "p", "d" },
	{ "q", "b" }, { "r", "n" }, { "s", "z" }, { "t", "m" },
	{ "u", "a" }, { "v", "p" }, { "w", "x" }, { "x", "l" },
	{ "y", "u" }, { "z", "z" },
	{ "", "" }
	}
        
    };

    if (lang == 0 || lang > NUM_LANGUAGES)
	return;

    lang -= 1;

    buf[0]      = '\0';
    for ( pName = argument; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(lang_syl_table[lang][iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( lang_syl_table[lang][iSyl].old, pName ) )
	    {
		strcat( buf, lang_syl_table[lang][iSyl].new );
		break;
	    }
	}

	if ( length < 1 )
	{
	    length = 1;
	    if (lang != 2) sprintf(buf,"%s%c",buf,pName[0]);
	}
    }


    if ( !IS_NPC(ch) )
    {
      if ( dice(1, 200) < ch->pcdata->condition[COND_DRUNK] )
      {
         strcat ( buf, "  I love you." );
      }

      if ( dice(1, 200) < ch->pcdata->condition[COND_DRUNK] )
      {
         strcat ( buf, "  I'm not drunk, you know." );
      } 

      if ( dice(1, 200) < ch->pcdata->condition[COND_DRUNK] )
      {
         strcat ( buf, "  I love everything." );
      }

      if ( dice(1, 500) < ch->pcdata->condition[COND_DRUNK] )
      {
         strcat ( buf, "  I'm not going to be sick." );
      }

    }

}

void do_dismount(CHAR_DATA *ch, char *argument)
{
    if (ch->riding != NULL)
    {
        act("You dismount $N.", ch, NULL, ch->riding, TO_CHAR);
        act("$n dismounts $N.", ch, NULL, ch->riding, TO_NOTVICT);
        act("$n dismounts you.", ch, NULL, ch->riding, TO_VICT);
        ch->riding->riden_by = NULL;
        ch->riding = NULL;
        return;
    }
    else
        send_to_char("You aren't riding anything.\n\r",ch);
}

void do_mount(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim=NULL;

    if (ch->secondaryrace == RACE_DRAGON)
    {
	send_to_char("You are a dragon, and cannot ride anything.\n\r",ch);
	return;
    }

    set_stance(ch, 0);

    if (argument[0]=='\0')
    {
	if (ch->riding != NULL)
	{
	    act("You dismount $N.", ch, NULL, ch->riding, TO_CHAR);
	    act("$n dismounts $N.", ch, NULL, ch->riding, TO_ROOM);
	    act("$n dismounts you.", ch, NULL, ch->riding, TO_VICT);
	    ch->riding = NULL;
	    victim->riden_by = NULL;
	    return;
	}
	send_to_char("Mount what?\n\r",ch);
	return;
    }

    if ((victim = get_char_room(ch, argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_NPC(victim) && !IS_SET(victim->act, ACT_RIDEABLE))
    {
	send_to_char("They don't look like they want to be ridden.\n\r",ch);
	return;
    }

    if (victim->riding != NULL)
    {
	send_to_char("But they are riding someone else.\n\r", ch);
	return;
    }

    if (ch->riding != NULL)
    {
	send_to_char("You are already on a mount.\n\r", ch);
	return;
    }

    if (ch->riden_by != NULL)
    {
	send_to_char("But you are currently being ridden.\n\r", ch);
	return;
    }

    if (victim->riden_by != NULL)
    {
	send_to_char("Someone is already riding them.\n\r",ch);
	return;
    }

    if ( (ch->secondaryrace == RACE_VAMPIRE && ch->power[DISC_VAMP_ANIM] <3)
	|| victim->master != ch )
    {
	act("$N backs away as you try to approach it.",ch, NULL, victim, TO_CHAR);
	return;
    }

    if (!IS_NPC(victim) && victim->secondaryrace != RACE_DRAGON)
    {
	send_to_char("You cannot mount them.\n\r",ch);
	return;
    }

    if (get_size(ch) > get_size(victim) + 2)
    {
	send_to_char("You'd squish them.\n\r",ch);
	return;
    }

    if (get_size(ch) > get_size(victim))
    {
	send_to_char("They don't look large enough to mount.\n\r",ch);
	return;
    }

    if (victim->position < POS_STANDING)
    {
	send_to_char("They have to stand up first.\n\r",ch);
	return;
    }

    do_group(ch, GET_NAME(victim));
    ch->riding = victim;
    victim->riden_by = ch;
    act("You mount $N.", ch, NULL, victim, TO_CHAR);
    act("$n mounts $N.", ch, NULL, victim, TO_NOTVICT);
    act("$n mounts you.", ch, NULL, victim, TO_VICT);
}

/*
void do_research(CHAR_DATA *ch, char *argument)
{
    int i = 0, sl, xpm, xpd, tmm, tmd, cost;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];

    struct research_type
    {
	char *name;
	int cost;
	int time;
    };

    int numslots = 29;

    static const struct research_type res_data[30] = 
    {
	{ "bash", 1, 24 },
	{ "dirt kicking", 1, 24 },
	{ "second attack", 2, 50},
	{ "parry", 2, 50},
	{ "disarm", 2, 50},
	{ "dodge", 2, 30},
	{ "envenom", 3, 72 },
	{ "rescue", 1, 24 },
	{ "trip", 2, 24 },
	{ "fast healing", 3, 72 },
	{ "haggle", 2, 48 },
	{ "lore", 2, 50 },
	{ "meditation", 3, 72 },
	{ "off hand", 2, 48 },
	{ "hand to hand", 1, 30 },
	{ "third attack", 3, 50 },
	{ "blind fighting", 3, 50 },
	{ "enhanced damage", 2, 50 },
	{ "axe", 2, 30 },
	{ "dagger", 2, 30 },
	{ "flail", 2, 30 },
	{ "mace", 2, 30 },
	{ "polearm", 2, 30 },
	{ "spear", 2, 30 },
	{ "sword", 2, 30 },
	{ "whip", 2, 30 },
	{ "tumbling", 2, 30 },
	{ "elvish", 3, 72 },
	{ "gnome", 3, 72 },
	{ "common", 3, 72 }
    };

    xpm = config_info->research_exp_mult;
    xpd = config_info->research_exp_div;
    tmm = config_info->research_time_mult;
    tmd = config_info->research_time_mult;

    buf[0] = '\0';

    if (IS_NPC(ch))
	return;

	if (ch->research_timer != -1)
	{
	    send_to_char("You are currently researching ",ch);
	    send_to_char(skill_table[ch->research_slot].name,ch);
	    if (ch->research_timer > 40)
		send_to_char(" and still have a lot to do.\n\r",ch);
	    else if (ch->research_timer > 20)
		send_to_char(" and will be quite some time.\n\r",ch);
	    else if (ch->research_timer > 10)
		send_to_char(" but are nearly through.\n\r",ch);
	    else if (ch->research_timer > 5)
		send_to_char(" but haven't too much to do.\n\r",ch);
	    else if (ch->research_timer > 1)
		send_to_char(" and are almost finished.\n\r",ch);
	    else
		send_to_char(" and are on the brink of finishing.\n\r",ch);
	    return;
	}

    if (argument[0]=='\0')
    {
    for (i = 0; i < numslots; i++)
    {
	sl = skill_lookup(res_data[i].name);
	if (ch->pcdata->learned[sl] < 1)
	{
	    cost = ( ((1000*ch->pcdata->things_researched) + 2000 * (ch->researched + 1) * res_data[i].cost) * xpm / xpd);
	    if (cost < 0 || cost > 100000) cost = 100000;
	    sprintf(buf, "    %16s will cost %d experience to research.\n\r",
		res_data[i].name,cost);
	    if (!found)
		send_to_char("You may research any one of the following:\n\r",ch);
	    send_to_char(buf, ch);
	    found = TRUE;
	}
    }
    if (!found)
	send_to_char("You have researched all available skills.\n\r",ch);
    return;
    }

    found = FALSE;

    for (sl = 0; sl < numslots; sl++)
    {
	if (!str_prefix(argument, res_data[sl].name))
	{
	    found = TRUE;
	    i = sl;
	    break;
	}
    }

    if (!found)
    {
	send_to_char("You know of no such skill you could research.\n\r",ch);
	return;
    }

    cost = ((1000*ch->pcdata->things_researched) + 2000 * (ch->researched + 1) * res_data[i].cost) * xpm / xpd;
    if (cost < 0 || cost > 100000) cost = 100000;

    if (ch->currentexp < cost)
    {
	sprintf(buf, "You need %d exp before you may research %s.\n\r",
	    cost,
	    res_data[i].name);
	send_to_char(buf, ch);
	return;
    }

    sprintf(buf, "You begin your research into %s.\n\r",
	res_data[i].name);
    send_to_char(buf, ch);

    sl = skill_lookup(res_data[i].name);

    ch->currentexp -= cost;

    ch->research_slot = sl;
    ch->research_timer = res_data[i].time * tmm / tmd;
    ch->researched++;
}
*/

void do_introduce(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i;
    bool sayit = TRUE;

    if (IS_NPC(ch))
	return;

    if (!TIME_UP(ch, TIMER_TAINT))
    {
	stc("You have no lineage.\n\r", ch);
	return;
    }

    if (!str_cmp(argument, "test"))
    {
	for (i = 0; i < 13; i++)
	{
	    sprintf(buf,"slot %-2d = %s~\n\r",
		i, (ch->pcdata->lineage[i] == NULL) ? "NULL" : ch->pcdata->lineage[i] );
	    send_to_char(buf, ch);
	}
	return;
    }

    if (!str_cmp(argument, "self"))
    {
	send_to_char("You recite your lineage to yourself....\n\r",ch);
	sayit = FALSE;
    }
    else
    {
	act("In true vampire tradition, you recite your lineage.",ch,NULL,NULL, TO_CHAR);
	act("$n recites $s lineage in the true vampire tradition.",ch, NULL,NULL,TO_ROOM);
    }

    for (i = 13; i >= 0; i--)
	if (ch->pcdata->lineage[i] != NULL) break;

    for (; i >= 0; i--)
    {
	sprintf(buf, "My name is %s, and my sire is %s.",
	    (i > 11) ? ch->pcdata->switchname : (ch->pcdata->lineage[i+1] == NULL) ? ch->pcdata->switchname : ch->pcdata->lineage[i+1],
	    ch->pcdata->lineage[i]);
	if (sayit)
	{
	    send_to_char("You say '",ch);
	    send_to_char(buf,ch);
	    send_to_char("'\n\r",ch);

	    sprintf(buf, "$n says 'My name is %s, and my sire is %s.'",
                (i > 11) ? ch->pcdata->switchname : (ch->pcdata->lineage[i+1] == NULL) ? ch->pcdata->switchname : ch->pcdata->lineage[i+1],
                ch->pcdata->lineage[i]);

	    act(buf, ch, NULL, NULL, TO_ROOM);
	}
	else 
	{
	    send_to_char(buf, ch);
	    send_to_char("\n\r",ch);
	}
    }
}

void do_autowalk( DESCRIPTOR_DATA *d )
{
    char buf[MAX_INPUT_LENGTH];
    int i, j, k;
    bool expand = TRUE;

    if (d->incomm[1] == '\0' && TIME_UP(d->character, TIMER_ON_SPEED) )
	return;

    if (!str_prefix("news", d->incomm))
	return;

    for (i = 0; d->incomm[i] != '\0'; i++)
    {
	if (!isdigit(d->incomm[i])
	    && d->incomm[i] != 'n'
	    && d->incomm[i] != 's'
	    && d->incomm[i] != 'e'
	    && d->incomm[i] != 'w'
	    && d->incomm[i] != 'u'
	    && d->incomm[i] != 'd') expand = FALSE;
	if (expand)
	    buf[i] = d->incomm[i];
    }
    buf[i] = '\0';

    if (!expand)
	return;

    SET_BIT(d->character->flag4, AFF_AUTOWALKING);

    j = 0;
    k = 0;
    for (i = 0; buf[i] != '\0'; i++)
    {
	if (isdigit(buf[i]))
	    k = k * 10 + buf[i] - '0';
	else
	{
	    if (k > 30) k = 30;
	    if (k == 0) k = 1;
	    if (!TIME_UP(d->character, TIMER_ON_SPEED)) k += 1;
	    for ( ; k > 0; k--)
	    {
		d->incomm[j++] = buf[i];
		d->incomm[j++] = ';';
	    }
	    k = 0;
	}
	if (j > MAX_INPUT_LENGTH - 10)
	    break;
    }
    d->incomm[j] = '\0';
}
