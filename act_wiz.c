#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hell.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help          );
DECLARE_DO_FUN(do_rstat         );
DECLARE_DO_FUN(do_ostat         );
DECLARE_DO_FUN(do_mstat         );
DECLARE_DO_FUN(do_rset          );
DECLARE_DO_FUN(do_mset          );
DECLARE_DO_FUN(do_oset          );
DECLARE_DO_FUN(do_sset          );
DECLARE_DO_FUN(do_dset          );
DECLARE_DO_FUN(do_mfind         );
DECLARE_DO_FUN(do_ofind         );
DECLARE_DO_FUN(do_slookup       );
DECLARE_DO_FUN(do_quit          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_stand         );
DECLARE_DO_FUN(do_oload         );
DECLARE_DO_FUN(do_mload         );
DECLARE_DO_FUN(do_visible       );
DECLARE_DO_FUN(do_think         );

int flag_lookup args( ( const char *name, const struct flag_type *flag_table) );
void    weather_update  args( ( void ) );
void    char_update     args( ( void ) );
void    obj_update      args( ( void ) );
void    area_update     args( ( void ) );
void    room_update	args( ( void ) );

/*
 * Local functions.
 */
ROOM_INDEX_DATA *       find_location   args( ( CHAR_DATA *ch, char *arg ) );

void do_component(CHAR_DATA *ch, char *argument)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
	stc("What spell do you wish to discover the component of?\n\r", ch);
	return;
    }

    for (i = 0; i < MAX_SKILL; i++)
    {
	if (skill_table[i].name != NULL && !str_prefix(argument,skill_table[i].name))
	{
	    if (skill_table[i].level > ch->power[DISC_WARL_LUCU] + 2)
	    {
		do_think(ch, "Hmmmm, it needs a component but im not sure what.");
		return;
	    }

	    if (skill_table[i].material_comp[0] == '\0')
	    {
		sprintf(buf,"I don't think %s requires a material component.",
			skill_table[i].name);
		do_think(ch, buf);
		return;
	    }
	    sprintf(buf, "I bet it needs %s to work.", 
			skill_table[i].material_comp);
	    do_think(ch, buf);
	    return;
	}
    }

    stc("Thats not a spell!\n\r", ch);
}
 
void do_animate(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    OBJ_DATA *obj;

    if (argument[0] == 0)
    {
	stc("Animate what?\n\r", ch);
	return;
    }

    if ((obj = get_obj_here(ch, argument))==NULL)
    {
	stc("You do not see that here.\n\r", ch);
	return;
    }

    mob = animate_object( obj );
    if (mob == NULL)
    {
	stc("Your powers have failed you.\n\r", ch);
	return;
    }

    act("$n shudders with life.", mob, NULL, NULL, TO_ROOM);
}

void do_make(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    int i;
    bool found = FALSE;

    if (argument[0] == '\0')
    {
	send_to_char("Syntax: make <spell name>, and Bobs your Uncle, you have the component\n\r", ch);
	return;
    }

    for (i = 0; i < MAX_SKILL; i++)
    {
	if (skill_table[i].name != NULL && !str_prefix(argument,skill_table[i].name))
	{
	    if (skill_table[i].material_comp[0] == '\0')
	    {
		send_to_char("Hey, dumbo.  That spell needs no material component!\n\r",ch);
		return;
	    }

	    obj = create_object( get_obj_index( 76 ), 0);
	    obj_to_char( obj, ch );
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( skill_table[i].material_comp );
	    free_string( obj->name );
	    obj->name = str_dup( skill_table[i].material_comp );
	    found = TRUE;
	    break;
	}
    }

    if (!found)
	send_to_char("You know of no such spell.\n\r",ch);
    else
	send_to_char("You've made it.  Hurray.  Horrah. Whoopie.\n\r",ch);
}

void do_proglist(CHAR_DATA *ch, char *argument)
{
    MOB_PROG_DATA *mp;
    int ln;

    send_to_char("Full Mob Programs Listings\n\r",ch);

    for (mp = mob_prog_list; mp != NULL; mp = mp->next)
    {
	send_to_char(mp->proc_name, ch);
	send_to_char("\n\r{",ch);
	for (ln = 0; mp->line[ln] != NULL; ln++)
	{
	    send_to_char("\n\r    ",ch);
	    send_to_char(mp->line[ln], ch);
	}
	send_to_char("\n\r}\n\r",ch);
    }
}


void do_cert( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    argument = one_argument(argument, buf);

    if (buf[0]=='\0')
    {
	send_to_char("Syntax: CERT <character> <enter room text>\n\r",ch);
	return;
    }

    if ((vch = get_char_world(ch, buf)) == NULL)
    {
	send_to_char("They aren't playing.\n\r",ch);
	return;
    }

    if (IS_NPC(vch))
    {
	send_to_char("You can only do this to PC's.\n\r",ch);
	return;
    }

    smash_tilde( argument );
    sprintf(buf, " %s", argument);
    free_string( vch->pcdata->custom_enter_room );
    vch->pcdata->custom_enter_room = str_dup( buf );
    look_name( vch );
}

void do_dataload(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    int count = 0;

    stc("Current mud data transfer loading on each descriptor:\n\r", ch);
    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d->character == NULL) continue;
	count += 1;
	stcf(ch, "    %3d. %20s - %d%d%d bytes in %d%d %d/4 seconds.\n\r",
		count, d->character->name, d->sent_bytes[2],
		d->sent_bytes[1], d->sent_bytes[0],
		d->sent_secs[1], d->sent_secs[0], d->sent_pulse);
    }
    stcf(ch, "%d descriptors found.\n\r", count);
}

void do_saylock(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0]=='\0')
    {
	send_to_char("SayLock who?\n\r",ch);
	return; 
    }

    if ((victim=get_char_world(ch, argument))==NULL)
    {
	send_to_char("They are not playing.\n\r",ch);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("PC's only please.\n\r",ch);
	return;
    }

    if (ch->level <= victim->level)
    {
	send_to_char("You must be of a higher level to SayLock them.\n\r",ch);
	return;
    }

    if (IS_FLAG3(victim, AFF_SAY_LOCK))
    {
	send_to_char("Removing SayLock.\n\r",ch);
	send_to_char("You have been released.\n\r",victim);
	free_string(victim->prefix);
	victim->prefix = str_dup("");
	REMOVE_BIT(victim->flag3, AFF_SAY_LOCK);
	return;
    }
    else
    {
	send_to_char("Setting SayLock.\n\r",ch);
	send_to_char("You have been forced into conversation.\n\r",victim);
	free_string(victim->prefix);
	victim->prefix = str_dup("say ");
	SET_BIT(victim->flag3, AFF_SAY_LOCK);
	return;
    }
}

void do_beep (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0]=='\0')
    {
	send_to_char("Beep who?\n\r",ch);
	return; 
    }

    if ((victim=get_char_world(ch, argument))==NULL)
    {
	send_to_char("They are not playing.\n\r",ch);
	return;
    }

    send_to_char("Beeping them.\n\r",ch);
    send_to_char("\7\7\7",victim);
}

void do_makecool (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	
    one_argument( argument, arg );
		
    if ( arg[0] == '\0' )
    {
	send_to_char( "Make who cool?\n\r", ch );
	return;
    }   
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    
    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }
    
    victim->max_hit = 30000;
    victim->max_move = 30000;
    victim->max_psp = 30000;
    victim->pcdata->perm_hit = 30000;
    victim->pcdata->perm_move = 30000; 
    victim->hit = 30000;
    victim->move = 30000;
    victim->qp = 30000;
    victim->faith = 30000;
    victim->blood = 30000;
    victim->psp = 30000;
    victim->exp = 300000;

    send_to_char("You feel like a new person.\n\r",victim);
    send_to_char("Your victim is now cool!\n\r",ch);
    act("$n shimmers with a golden aura.",victim, NULL, NULL, TO_ROOM);
    return;
}

void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' ) 
    {
	if (IS_SET(ch->wiznet,WIZ_ON))
	{
	    send_to_char("Signing off of Wiznet.\n\r",ch);
	    REMOVE_BIT(ch->wiznet,WIZ_ON);
	}
	else
	{
	    send_to_char("Welcome to Wiznet!\n\r",ch);
	    SET_BIT(ch->wiznet,WIZ_ON);
	}
	return;
    }

    if (!str_cmp(argument,"all") && ch->level == 120)
    {
	send_to_char("Full Wiznet priviliges invoked!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	SET_BIT(ch->wiznet,WIZ_LEVELS);
	SET_BIT(ch->wiznet,WIZ_LINKS);
	SET_BIT(ch->wiznet,WIZ_SITES);
	SET_BIT(ch->wiznet,WIZ_SNOOPS);
	SET_BIT(ch->wiznet,WIZ_SECURE);
	SET_BIT(ch->wiznet,WIZ_DEATHS);
	SET_BIT(ch->wiznet,WIZ_PREFIX);
	SET_BIT(ch->wiznet,WIZ_NEWBIE);
	SET_BIT(ch->wiznet,WIZ_SPAM);
	SET_BIT(ch->wiznet,WIZ_RESTORE);
	SET_BIT(ch->wiznet,WIZ_FLAGS);
	SET_BIT(ch->wiznet,WIZ_SWITCHES);
	SET_BIT(ch->wiznet,WIZ_LOAD);
    	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Welcome to Wiznet!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Signing off of Wiznet.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Wiznet status:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Wiznet options available to you are:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }
    else
    {
	sprintf(buf,"You will now see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
	{
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
		send_to_char("--> ",d->character);
	    act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
	}
	else if (d->connected == CON_PLAYING
	&& d->character != ch
	&& IS_FLAG5(d->character, AFF5_IMMTRUST)
	&& flag == WIZ_NEWBIE)
	{
	    stc("#1--#3==#7> ",d->character);
	    act_new(string, d->character, obj, ch, TO_CHAR, POS_DEAD);
	}
    }
 
    return;
}

void do_guild( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: guild <char> <clan name>\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't playing.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	stc("Only players may join a clan.\n\r", ch);
	return;
    }
    
    if (!str_prefix(arg2,"none"))
    {
	send_to_char("They are now clanless.\n\r",ch);
	send_to_char("You are now a member of no clan!\n\r",victim);
	victim->clan = 0;
	return;
    }

    if ((clan = clan_lookup(arg2)) == 0)
    {
	send_to_char("No such clan exists.\n\r",ch);
	return;
    }

    if (clan_table[clan].independent)
    {
	sprintf(buf,"They are now a %s.\n\r",clan_table[clan].name);
	send_to_char(buf,ch);
	sprintf(buf,"You are now a %s.\n\r",clan_table[clan].name);
	send_to_char(buf,victim);
    }

    else
    {
	sprintf(buf,"They are now a member of clan %s.\n\r",
	    capitalize(clan_table[clan].name));
	send_to_char(buf,ch);

	sprintf(buf,"You are now a member of clan %s.\n\r",
	    capitalize(clan_table[clan].name));
	if (ch != victim)
	    send_to_char(buf, victim);
    }

    victim->power[clan_table[victim->clan].disc1] = -1;
    victim->power[clan_table[victim->clan].disc2] = -1;
    victim->power[clan_table[victim->clan].disc3] = -1;

    victim->clan = clan;
    set_clan_disciplines(victim);
    stc("Clan disciplines have been modified - use induct in future.\n\r", ch);
}

void do_sprout(CHAR_DATA *ch, char *argument)
{
    REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);
    REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);
    REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);
    REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);
    SET_BIT(ch->body_form, BODY_ENABLE_THIRD_ARM);
    SET_BIT(ch->body_form, BODY_ENABLE_FOURTH_ARM);
    SET_BIT(ch->body_form, BODY_THIRD_ARM);
    SET_BIT(ch->body_form, BODY_THIRD_HAND);
    SET_BIT(ch->body_form, BODY_FOURTH_ARM);
    SET_BIT(ch->body_form, BODY_FOURTH_HAND);

    send_to_char("You should now have 4 arms.\n\r",ch);
}

     
void do_immtrust( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
 
    if (strcmp(ch->name, "Mitzrah") != 0) 
    { send_to_char("Sorry, you cannot do that.\n", ch);
      return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Immtrust whom?", ch );
	return;
    }
 
    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_FLAG5(victim, AFF5_IMMTRUST)) 
    {
	stc("Immtrust removed.\n\r", ch);
	REMOVE_BIT(victim->flag5, AFF5_IMMTRUST);
	return;
    }
    stc("Immtrust set.\n\r", ch);
    stc("You are now trusted as a spammer-killer.\n\r", victim);
    stc("All imm commands are logged, so if you abuse this privilige.\n\r", victim);
    stc("    in any way you will be spanked harshly.\n\r", victim);
    SET_BIT(victim->flag5, AFF5_IMMTRUST);
}


void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    if (ch->level < 112 && !IS_SET(ch->flag5, AFF5_IMMTRUST) )
    {
	stc("Huh?\n\r", ch);
	return;
    }

    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	send_to_char( "Nochannel whom?", ch );
	return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
	REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
	send_to_char( "The gods have restored your channel priviliges.\n\r", 
		      victim );
	send_to_char( "NOCHANNELS removed.\n\r", ch );
	sprintf(buf,"$N restores channels to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOCHANNELS);
	send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		       victim );
	send_to_char( "NOCHANNELS set.\n\r", ch );
	sprintf(buf,"$N revokes %s's channels.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}


void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch || vch->altitude != ch->altitude)
	    continue;
 
	if ((letter = strstr(argument,vch->name)) == NULL)
	{
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
	    continue;
	}
 
	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
	last[0] = '\0';
	name = vch->name;
 
	for (; *letter != '\0'; letter++)
	{
	    if (*letter == '\'' && matches == strlen(vch->name))
	    {
		strcat(temp,"r");
		continue;
	    }
 
	    if (*letter == 's' && matches == strlen(vch->name))
	    {
		matches = 0;
		continue;
	    }
 
	    if (matches == strlen(vch->name))
	    {
		matches = 0;
	    }
 
	    if (*letter == *name)
	    {
		matches++;
		name++;
		if (matches == strlen(vch->name))
		{
		    strcat(temp,"you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}
		strncat(last,letter,1);
		continue;
	    }
 
	    matches = 0;
	    strcat(temp,last);
	    strncat(temp,letter,1);
	    last[0] = '\0';
	    name = vch->name;
	}
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL && ch->level <120 )
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

	sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
	send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );
 
	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
	    send_to_char(buf,ch);
	    return;
	}
 
	if ( strstr(argument,ch->name) == NULL && ch->level < 120)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
 
	free_string( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
 
	sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
	send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    if (ch->level < 112 && !IS_SET(ch->flag5, AFF5_IMMTRUST) )
    {
	stc("Huh?\n\r", ch);
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->descriptor == desc)
	    {
		close_socket( d );
		send_to_char( "Ok.\n\r", ch );
		return;
	    }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_CREATOR(victim)) 
    {
	send_to_char("Pull the other one!\n\r",ch);
	act("$N just tried to disconnect you!  Tut tut tut!",victim,NULL,ch,TO_CHAR);
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if (!IS_GRANTED(ch, CMD_ECHO) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if (!IS_GRANTED(ch, CMD_ECHO) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (!IS_GRANTED(ch, CMD_ECHO) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if (!IS_GRANTED(ch, CMD_ECHO) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
	send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( char_realm(ch), atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return locate_obj(obj);

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    int alt = 0;

    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
        alt = ch->altitude;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    }
    char_from_room( victim );
    char_to_room( victim, location, 0 );
    victim->altitude = alt;
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    }
    if (victim->position > POS_SLEEPING)
    {
	do_look( victim, "auto" );
    }
    send_to_char( "Ok.\n\r", ch );
    check_fall(victim);
}

void gecho(char *message)
{
    CHAR_DATA *wch;

    if (message[0] == '\0') return;

    for ( wch = char_list; wch != NULL; wch = wch->next )
	stc(message, wch);
}

void gecho_outside(char *message)
{
    CHAR_DATA *wch;

    if (message[0] == '\0') return;

    for ( wch = char_list; wch != NULL; wch = wch->next )
	if (IS_OUTSIDE(wch)) stc(message, wch);
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    int oldalt;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    oldalt = ch->altitude;
    if ((wch = get_char_world(ch, arg)) != NULL) ch->altitude = wch->altitude; 
    char_from_room( ch );
    char_to_room( ch, location, ch->altitude );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original, ch->altitude );
	    ch->on = on;
	    ch->altitude = oldalt;
	    break;
	}
    }

    return;
}

void do_goto_object( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    OBJ_DATA *obj;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto which object?\n\r", ch );
	return;
    }

    if ( (obj = get_obj_world(ch, argument)) == NULL)
    {
	send_to_char("No such object exists.\n\r", ch);
	return;
    }


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->altitude != ch->altitude) continue;
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }
    }

    location = locate_obj(obj);

    char_from_room( ch );
    char_to_room( ch, location, ch->altitude );
    ch->altitude = obj->altitude;


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->altitude != ch->altitude) continue;
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
		act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
	    else
		act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }
    }

    do_look( ch, "auto" );
    return;
}




void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->altitude != ch->altitude) continue;
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }
    }

    char_from_room( ch );
    char_to_room( ch, location, 0 );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->altitude != ch->altitude) continue;

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
		act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
	    else
		act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }
    }

    do_look( ch, "auto" );
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!room_is_private( location ))
    {
	send_to_char( "That room isn't private, use goto.\n\r", ch );
	return;
    }
 
    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->altitude != ch->altitude) continue;
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }
    }
 
    char_from_room( ch );
    char_to_room( ch, location, ch->altitude );
 
    ch->altitude = 0;

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->altitude != ch->altitude) continue;
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
		act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
	    else
		act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }
    }
 
    do_look( ch, "auto" );
    return;
}

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }
   
   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

   



void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door, i;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d Gore : %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->gore );
    send_to_char( buf, ch );

    sprintf(buf, "Timers ");
    for (i = 0 ; i < MAX_RTIMER ; i++)
    {
	sprintf(buf2, "%d ", RTIMER(location, i));
	strcat(buf, buf2);
    }
    wrap_to_char(buf2, ch);

    sprintf( buf,
	"Room flags: %d.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
	{
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
		pexit->key,
		pexit->exit_info,
		pexit->keyword,
		pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Voodoo link: %s\n\r",
	obj->voodoo_name );
    send_to_char( buf, ch );

    sprintf( buf, "Last Owner: %s\n\r",
	obj->last_owner );
    send_to_char( buf, ch );



    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_table[obj->item_type].name, obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
					: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
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

        case ITEM_SPEC_POTION:
	    sprintf(buf, "Its a special potion, of type %d!", obj->value[1]);
	    send_to_char( buf, ch );
	break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
	    sprintf( buf, "Has %d(%d) charges of level %d",
		obj->value[1], obj->value[2], obj->value[0] );
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
		
      
	case ITEM_WEAPON:
	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
		case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
		case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;      
		case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
		case(WEAPON_SPEAR):
		    send_to_char("spear/staff\n\r",ch);
		    break;
		case(WEAPON_MACE): 
		    send_to_char("mace/club\n\r",ch);   
		    break;
		case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch); 
		    break;
		case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
		case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
		case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
		default: 
		    send_to_char("unknown\n\r",ch);
		    break;
	    }
	    if (obj->pIndexData->new_format)
		sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
		sprintf( buf, "Damage is %d to %d (average %d)\n\r",
		    obj->value[1], obj->value[2],
		    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
		    attack_table[obj->value[3]].noun : "undefined");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
		sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
		send_to_char(buf,ch);
	    }
	break;

	case ITEM_ARMOR:
	    sprintf( buf, 
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
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
    }

    if ( obj->spec_proc != 0 )
    {
	sprintf(buf, "Special procedure: %d\n\r",obj->spec_proc);
	send_to_char( buf,ch);
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    break;
		case TO_WEAPON:
		    sprintf(buf,"Adds %s weapon flags.\n",
			weapon_bit_name(paf->bitvector));
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
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
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
	    send_to_char(buf,ch);
	}
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int i;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: %s\n\r",
	victim->name);
    send_to_char( buf, ch );

    sprintf( buf, 
	"Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    crash("mstat a horse");

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf, 
	"Str: %d(%d) Int: %d(%d) Wis: %d(%d) Dex: %d(%d) Con: %d(%d) Cha: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON),
	victim->perm_stat[STAT_CHA],
	get_curr_stat(victim,STAT_CHA) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Vitality: %d/%d\n\r",
	victim->hit,         victim->max_hit,
	victim->move,        victim->max_move);
    send_to_char( buf, ch );
	
    sprintf( buf,
	"Lv: %d  Align: %d  Gold: %ld  Silver: %ld  Exp: %d\n\r",
	victim->level,       
	victim->alignment,
	victim->gold, victim->silver, victim->exp );
    send_to_char( buf, ch );

    crash("mstat a bamboo hat");

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[get_size(victim)].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char( buf, ch );

    send_to_char("Secondary race: ",ch);
    switch (victim->secondaryrace)
    {
	case 0 : send_to_char("None",ch);
		 break;
	case 1 : send_to_char("Dragon",ch);
		 break;
	case 2 : send_to_char("Daemon",ch);
		 break;
	case 3 : send_to_char("Vampire",ch);
		 break;
	case 4 : send_to_char("Lupine",ch);
		 break;
	case 5 : send_to_char("Eladrin",ch);
		 break;
	case 6 : send_to_char("Warlock",ch);
		 break;
	case 7 : send_to_char("Hero",ch);
		 break;
	case 8 : send_to_char("Death Knight",ch);
		 break;
	case 9 : send_to_char("Psionicist",ch);
		 break;
	default: send_to_char("BUG! Bzzzzzz Damn those blasted bugs",ch);
		 break;
    }
    send_to_char("\n\r",ch);

    crash("mstat a lard trail");

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK] );
	send_to_char( buf, ch );
    }

    crash("mstat a jumping man");

    sprintf( buf, "Carry number: %d  Carry weight: %ld\n\r",
	victim->carry_number, get_carry_weight(victim) / 10 );
    send_to_char( buf, ch );


    if (!IS_NPC(victim))
    {
	sprintf( buf, 
	    "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
	send_to_char( buf, ch );
    }

    sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);
    
    crash("mstat a jumping woman");

    if (victim->comm)
    {
	sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affected by %s\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    crash("mstat your own will");

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet         ? victim->pet->name      : "(none)");
    send_to_char( buf, ch );

    crash("mstat beans on toast");

    sprintf( buf, "Short description: %s\n\r",
	victim->short_descr);
    send_to_char( buf, ch );

    crash("mstat a dogs dinner");

    if (ch->pIndexData != NULL)
    {
        for ( i = 0; i < NUM_TRIGS; i++)
        {
	    if (victim->pIndexData->trig[i] != NULL)
	    {
	        stcf(ch, "Mob prog '%s' attached to trigger %d.\n\r",
		  victim->pIndexData->trig[i]->proc_name, i);
  	    }
        }
    }
    crash("mstat your daughters throat");

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    if (!IS_GRANTED(ch, CMD_LOAD) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll        = FALSE; /* !str_cmp( arg, "all" ); */
    found       = FALSE;
    nMatch      = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll        = FALSE; /* !str_cmp( arg, "all" ); */
    found       = FALSE;
    nMatch      = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj_no_alt( ch, obj ) || !is_name( argument, obj->name )
	||   ch->level < obj->level)
	    continue;
 
	found = TRUE;
	number++;
 
	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;
 
	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
	    sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
		number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
	else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
	    sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
		number, obj->short_descr,in_obj->in_room->name, 
		in_obj->in_room->vnum);
	else
	    sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
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
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	buffer = new_buf();
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		else
		    sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		add_buf(buffer,buf);
	    }
	}

	page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
	if ( victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    if (!IS_GRANTED(ch, CMD_REBOOT) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (!IS_GRANTED(ch, CMD_REBOOT) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if (ch->invis_level < LEVEL_HERO)
    {
	sprintf( buf, "Reboot by %s.  Game will be back up in 5 seconds, dont go away...", ch->name );
	do_echo( ch, buf );
    }

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
        {
	    vch->riding = NULL;
	    vch->riden_by = NULL;
	    if (is_item(vch)) char_from_obj(vch);
	    save_char_obj(vch);
	}
	close_socket(d);
    }
    
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    if (!IS_GRANTED(ch, CMD_REBOOT) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (!IS_GRANTED(ch, CMD_REBOOT) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "Shutdown by %s.  Have a nice day.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
	do_echo( ch, buf );
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
        {
	    vch->riding = NULL;
	    vch->riden_by = NULL;
	    if (is_item(vch)) char_from_obj(vch);
	    save_char_obj(vch);
	}
	close_socket(d);
    }
    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    
    if ( victim->desc->snoop_by != NULL )
    {
	if (victim->desc->snoop_by == ch->desc)
	{
	    send_to_char("You stop snooping on them.\n\r",ch);
	    victim->desc->snoop_by = NULL;
	    sprintf(buf, "$N quits snooping on %s",
		(IS_NPC(ch) ? victim->short_descr : victim->name));
	    wiznet(buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE,get_trust(ch));
	    return;
	}

	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( (get_trust( victim ) >= get_trust( ch ) 
	|| (IS_SET(victim->comm,COMM_SNOOP_PROOF)))
        && str_cmp(GET_PROPER_NAME(ch),"Jobe")
        && str_cmp(GET_PROPER_NAME(ch),"Soder")
        && str_cmp(GET_PROPER_NAME(ch),"Mitzrah"))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    if (!IS_CREATOR(ch) )
    {
        sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
        wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    }
    send_to_char( "Snooping...\n\r", ch );
    return;
}

void do_as( CHAR_DATA *ch, char *argument )
{
   /* reimplemented by whois, it was not in our stock code */
    /* 
    char arg[MAX_INPUT_LENGTH];
    char * arg2;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    */
    return;
}

bool is_mob_switched(CHAR_DATA *ch)
{
    if ( IS_SET(ch->flag5, AFF5_SWITCHED) ) return TRUE;
    return FALSE;
}

void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( is_mob_switched(ch) )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( is_item(ch) )
    {
	stc("Not while in item form.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if (!IS_GRANTED(ch, CMD_SWITCH) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You cannot switch with yourself.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "That body is already inhabited.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    SET_BIT(victim->flag5, AFF5_SWITCHED);
    SET_BIT(ch->flag5, AFF5_SWITCHED);

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
	victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "You take over their body.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if (!is_item(ch) && !is_mob_switched(ch))
    {
	stc("You are not in the form of an object or a mob.\n\r", ch);
	return;
    }

    if (is_item(ch))
    {
	obj = ch->sw_obj;
	char_from_obj(ch);
	act("You return from $p to your original body.", ch, obj, NULL, TO_CHAR);
	if (IS_OBJ_STAT3(obj, ITEM_DESTROY_CHAR_RETURN))
	    act("$p suddenly transforms into $n.", ch, obj, NULL, TO_ROOM);
	else
	    act("A fine white mist seeps out of $p, forming into $n.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if ( ch->desc == NULL )
	return;

    if ( !is_mob_switched(ch) )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( "You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );

    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }
    REMOVE_BIT(ch->desc->character->flag5, AFF5_SWITCHED);
    REMOVE_BIT(ch->desc->original->flag5, AFF5_SWITCHED);
    REMOVE_BIT(ch->flag5, AFF5_SWITCHED);
    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    REMOVE_BIT(ch->flag5, AFF5_SWITCHED);
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI)     && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    if (!IS_GRANTED(ch, CMD_LOAD) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room, ch->altitude);
	recursive_clone(ch,obj,clone);

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room, ch->altitude);
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	act("$n has created $N.",ch,NULL,clone,TO_ROOM);
	act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }
	return;
    }
}

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    if (!IS_GRANTED(ch, CMD_LOAD) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room, ch->altitude );
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    act( "You have created $N!", ch, NULL, victim, TO_CHAR );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
	{
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
	level = atoi(arg2);
	if ((level < 0 || level > get_trust(ch)) && ch->level != 120)
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, (level == 666) ? 120 : level );
    obj->spec_proc = pObjIndex->spec_proc;
    if ( CAN_WEAR(obj, ITEM_TAKE) && level != 666 )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room, ch->altitude );
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    act( "You have created $p!", ch, obj, NULL, TO_CHAR );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    }
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	if (victim->altitude != ch->altitude) continue;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
	    {
		if (victim->riden_by != NULL)
		{
		    victim->riden_by->riding = NULL;
		    victim->riden_by = NULL;
		}
		else if (victim->riding != NULL)
		{
		    victim->riding->riden_by = NULL;
		    victim->riding = NULL;
		}
  		extract_char( victim, TRUE );
	    }
	}

	ch->in_room->gold = 0;
	ch->in_room->silver = 0;
	ch->in_room->emeralds = 0;
	ch->in_room->diamonds = 0;
	ch->in_room->sapphires = 0;
	ch->in_room->copper = 0;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
    }
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);
    }
	if (victim->level > 1)
	    save_char_obj( victim );
	d = victim->desc;
	extract_char( victim, TRUE );
	if ( d != NULL )
	  close_socket( d );

	return;
    }

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    }
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 1 to 120.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */

    if ( level <= victim->level )
    {
	send_to_char( "Lowering a player's level!\n\r", ch );
	victim->level    = 1;
	victim->hit      = victim->max_hit;
	victim->move     = victim->max_move;
	advance_level( victim, TRUE );
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
    }
	advance_level( victim,TRUE);
    /*send_to_char(buf,victim);*/
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}

void restore_char(CHAR_DATA *ch)
{
    affect_strip(ch,gsn_plague);
    affect_strip(ch,gsn_poison);
    affect_strip(ch,gsn_blindness);	   
    affect_strip(ch,gsn_sleep);
    affect_strip(ch,gsn_curse);
    affect_strip(ch,gsn_conflaguration);
    affect_strip(ch,gsn_fear); 
    affect_strip(ch,gsn_shiver);
    affect_strip(ch,gsn_mummy_rot);
    affect_strip(ch,gsn_blood_spew);
    affect_strip(ch,gsn_holdperson);
    affect_strip(ch,gsn_pestilence);
    ch->hit    = ch->max_hit;
    ch->move   = ch->max_move;
    ch->psp    = ch->max_psp;
    if (!IS_NPC(ch) )
    {
	ch->pcdata->condition[COND_THIRST] = 100;
	ch->pcdata->condition[COND_HUNGER] = 100;
    }
    ch->position = POS_STANDING;
    REMOVE_BIT(ch->body_form, BODY_LEFT_ARM);
    REMOVE_BIT(ch->body_form, BODY_RIGHT_ARM);
    REMOVE_BIT(ch->body_form, BODY_LEFT_HAND);
    REMOVE_BIT(ch->body_form, BODY_RIGHT_HAND);
    ch->blood = max_blood_pool(ch);
    update_pos(ch);
}

void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
	
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    if (vch->altitude != ch->altitude) continue;
	    restore_char(vch);
	    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
   	    {
	        act("$n has restored you.",ch,NULL,vch,TO_VICT);
    	    }
	}

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
	sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
	wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    }        
	send_to_char("Room restored.\n\r",ch);
	return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
	
	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
	    restore_char(d->character);		

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
		act("$n has restored you.",ch,NULL,victim,TO_VICT);
    }
	}
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    restore_char(ch);

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    }
    send_to_char( "Ok.\n\r", ch );
    return;
}

	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_smite( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
	
    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );
		
    if ( arg[0] == '\0' )
    {
	send_to_char( "Smite whom?\n\r", ch );
	return;
    }   
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    
    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }
    
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
    lose_body_part(victim, BODY_LEFT_HAND);

    victim->hit = victim->hit / 3;
    victim->move = 0;

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    send_to_char("You scream and writhe in agony as a thunderbolt sent by the gods from\n\rthe heavens strikes you down, crippling and mortally wounding you.\n\r",victim);
    send_to_char("You smite them, sending the wrath of the gods.\n\r",ch);
    act("$n drops to $s knees, crippled by the wrath of the gods.",victim, NULL, NULL, TO_ROOM);
    save_char_obj( victim );
    }    
    return;
}




void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (!IS_GRANTED(ch, CMD_CHANNELS) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (!IS_GRANTED(ch, CMD_CHANNELS) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (!IS_GRANTED(ch, CMD_CHANNELS) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if (rch->altitude != ch->altitude) continue;
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_elad_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    ROOM_INDEX_DATA *room;

    if (IS_EVIL(ch))
    {
	stc("You are too evil to use this power.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type != SECT_FOREST)
    {
	stc("You must be in the forest to use this power.\n\r", ch);
	return;
    }

    if (ch->hit < ch->max_hit)
    {
	stc("You must be at maximum health to use this power.\n\r", ch);
	return;
    }

    ch->hit /= 2;
    WAIT_STATE(ch, 10);

    for ( room = room_list ; room != NULL ; room = room->next )
    {
	if (room->sector_type != SECT_FOREST) continue;
        for ( rch = room->people; rch != NULL; rch = rch->next_in_room )
        {
	    /* no altitude check */
	    if ( rch->fighting != NULL )
	        stop_fighting( rch, TRUE );
	    if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	        REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
        }
	room_message(room, "The forest gives you an overwhelming sense of peace.");
    }
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if (!IS_GRANTED(ch, CMD_LOCK) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if ( wizlock )
    {
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if (!IS_GRANTED(ch, CMD_LOCK) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
	send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
	send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if (!IS_GRANTED(ch, CMD_LOAD) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set day   <value>\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
	send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"day") )
    {
        do_dset(ch, argument);
	return;
    }


    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}
void do_dset(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];
        int value;
        argument = one_argument( argument, arg );
	if (arg[0] == '\0')
	{
	   do_set(ch, "");
	   return;
        }

        value = atoi( arg );
        if ( value < 0 || value > 35 )
        {
	    send_to_char( "Value range is 0 to 35.\n\r", ch );
	    return;
        }
	time_info.day = value;
    	send_to_char("Day of the month has been set.\n\r", ch);
	return;
}

void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]     = value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",                      ch );
	send_to_char( "    str int wis dex con cha sex class level psp disci voodoo dragtype\n\r",ch );
	send_to_char( "    race group gold silver hp move faith tertrace primary\n\r",ch);
	send_to_char( "    align exp thirst hunger drunk full v0-v4 kai secrace gen\n\r",ch );
	send_to_char( "    copper emeralds sapphires diamonds\n", ch);
	send_to_char( "    newquest\n", ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 1 || value > 30 )
	{
	    sprintf(buf, "Strength range is 1 to 30\n\r.");
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_prefix( arg2, "dragtype" ) )
    {
	if ( value < 1 || value > 8 )
	{
	    sprintf(buf, "Dragtype range is 1 to 8\n\r.");
	    send_to_char(buf,ch);
	    return;
	}

	victim->dragtype = value;
	return;
    }

    if ( !str_prefix( arg2, "primary" ) )
    {
	if ( value < 0 || value > 6 )
	{
	    sprintf(buf, "Primary range is 0 to 6\n\r.");
	    send_to_char(buf,ch);
	    return;
	}

	victim->primary = value;
	return;
    }

    if ( !str_prefix( arg2, "voodoo" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
        }

	if ( value < 0 || value > 3000 )
	{
	    sprintf(buf, "Primary range is 0 to 3000\n\r.");
	    send_to_char(buf,ch);
	    return;
	}

	victim->pcdata->voodoo_points = value;
	return;
    }

    if ( !str_prefix( arg2, "newquest" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Not on NPC's.\n\r", ch);
	    return;
        }

	initialise_quest(victim);
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
	if ( value < 1 || value > 30 )
	{
	    sprintf(buf,
		"Intelligence range is 3 to 30.\n\r");
	    send_to_char(buf,ch);
	    return;
	}
 
	victim->perm_stat[STAT_INT] = value;
	return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	{
	if ( value < 1 || value > 30 )
	    sprintf(buf,
		"Wisdom range is 3 to 30.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "cha" ) )
    {
	if ( value < 1 || value > 30 )
	{
	    sprintf(buf,
		"Charisma range is 3 to 30.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CHA] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 1 || value > 30 )
	{
	    sprintf(buf,
		"Dexterity range is from 3 to 30.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 1 || value > 30 )
	{
	    sprintf(buf,
		"Constitution range is 3 to 30.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim) && victim->desc != NULL)
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    send_to_char( "Level range is 0 to 120.\n\r", ch );
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }
    if ( !str_prefix( arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }
    if ( !str_prefix( arg2, "copper" ) )
    {
	victim->copper = value;
	return;
    }
    if ( !str_prefix( arg2, "diamonds" ) )
    {
	victim->diamonds = value;
	return;
    }
    if ( !str_prefix( arg2, "sapphires" ) )
    {
	victim->sapphires = value;
	return;
    }
    if ( !str_prefix( arg2, "emeralds" ) )
    {
	victim->emeralds = value;
	return;
    }

    if ( !str_cmp( arg2, "psp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Psp range is -10 to 30,000 points.\n\r", ch );
	    return;
	}
	victim->max_psp = value;
	return;
    }

    if ( !str_cmp( arg2, "v0" ) )
    {
	victim->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "v1" ) )
    {
	victim->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "v2" ) )
    {
	victim->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "v3" ) )
    {
	victim->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "v4" ) )
    {
	victim->value[4] = value;
	return;
    }
    if ( !str_cmp( arg2, "v5" ) )
    {
	victim->value[5] = value;
	return;
    }
    if ( !str_cmp( arg2, "v6" ) )
    {
	victim->value[6] = value;
	return;
    }
    if ( !str_cmp( arg2, "v7" ) )
    {
	victim->value[7] = value;
	return;
    }
    if ( !str_cmp( arg2, "8" ) )
    {
	victim->value[8] = value;
	return;
    }
    if ( !str_cmp( arg2, "v9" ) )
    {
	victim->value[9] = value;
	return;
    }
    if ( !str_cmp( arg2, "v10" ) )
    {
	victim->value[10] = value;
	return;
    }

    if ( !str_prefix(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
	if (!IS_NPC(victim))
	    victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "generation" ) )
    {
	if ( value < 1 || value > 13 )
	{
	    send_to_char( "Generation range is from 1 to 13.\n\r", ch );
	    return;
	}
	victim->generation = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
	if (!IS_NPC(victim))
	    victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "faith" ) )
    {
	if ( value < 0 || value > 25000 )
	{
	    send_to_char( "Faith range is 0 to 25000 points.\n\r", ch );
	    return;
	}
	victim->faith = value;
	return;
    }

    if ( !str_prefix( arg2, "kai" ) )
    {
	if ( value < 0 || value > 25000 )
	{
	    send_to_char( "Kai range is 0 to 25000 points.\n\r", ch );
	    return;
	}
	victim->qp = value;
	return;
    }

    if ( !str_prefix( arg2, "secrace" ) )
    {
	if ( value < 0 || value > 9 )
	{
	    send_to_char( "Race can be from 0 to 9.\n\r", ch );
	    return;
	}
	victim->secondaryrace = value;
	SET_BIT(victim->flag5, AFF5_WHO_LIST);
	return;
    }

    if ( !str_prefix( arg2, "tertrace" ) )
    {
	if ( value < 0 || value > 8 )
	{
	    send_to_char( "Race can be from 0 to 8.\n\r", ch );
	    return;
	}
	victim->tertiaryrace = value;
	return;
    }

    if ( !str_prefix( arg2, "exp" ))
    {
	if (value < 0 || value > 200000000 )
	{
	    send_to_char("Exp range is 0 to 200000000 points.\n\r",ch);
	    return;
	}
	victim->currentexp = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}
 
	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Full range is -1 to 100.\n\r", ch );
	    return;
	}
 
	victim->pcdata->condition[COND_HUNGER] = value;
	return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }


    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if (!IS_GRANTED(ch, CMD_LOAD) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec prefix\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

	if ( !str_prefix( arg2, "name" ) )
	{
	    if ( !IS_NPC(victim) )
	    {
		send_to_char( "Not on PC's.\n\r", ch );
		return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
	}
	
	if ( !str_prefix( arg2, "description" ) )
	{
	    free_string(victim->description);
	    victim->description = str_dup(arg3);
	    return;
	}

	if ( !str_prefix( arg2, "short" ) )
	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
	}

	if ( !str_prefix( arg2, "long" ) )
	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
	}

	if ( !str_prefix( arg2, "title" ) )
	{
	    if ( IS_NPC(victim) )
	    {
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	    }

	    set_title( victim, arg3 );
	    return;
	}

	if ( !str_prefix( arg2, "prefix" ) )
	{
	    if ( IS_NPC(victim) )
	    {
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	    }

	    set_title_prefix( victim, arg3 );
	    return;
	}

    }
    
    if (!str_prefix(type,"object"))
    {
	/* string an obj */
	
	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
	}
	
	if ( !str_prefix( arg2, "name" ) )
	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
	}

	if ( !str_prefix( arg2, "short" ) )
	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
	}

	if ( !str_prefix( arg2, "long" ) )
	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
	}

	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
		send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
		return;
	    }

	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword         = str_dup( arg3     );
	    ed->description     = str_dup( argument );
	    ed->next            = obj->extra_descr;
	    obj->extra_descr    = ed;
	    return;
	}
    }
    
	
    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",                               ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",      ch );
	send_to_char("    extra wear level weight cost timer\n\r",              ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	&& str_prefix(arg2, "questcentre") )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",         ch );
	send_to_char( "    flags sector questcentre\n\r",    ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That room is private right now.\n\r",ch);
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if (!str_prefix(arg2, "questcentre"))
    {
	SET_BIT(location->room_flags2, ROOM_QUEST_CENTRE);
	return;
    }

    if ( !str_prefix( arg2, "flags" ) )
    {

	location->room_flags    = value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type   = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}



void do_sockets( CHAR_DATA *ch, char *argument )
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char state[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;

    if (IS_NPC(ch)) return;
    if (!IS_GRANTED(ch, CMD_CONNECTIONS) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    count       = 0;
    buf[0]      = '\0';

    one_argument(argument,arg);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->character != NULL && can_see( ch, d->character ) 
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name))))
	{
	    count++;
	    switch (d->connected)
	    {
		default 			: sprintf(state, "Unknown"); break;
		case CON_PLAYING		: sprintf(state, "Playing"); break;
		case CON_GET_NAME		: sprintf(state, "GetName"); break;
		case CON_GET_OLD_PASSWORD	: sprintf(state, "Passwrd"); break;
		case CON_CONFIRM_NEW_NAME	: sprintf(state, " Newbie"); break;
		case CON_GET_NEW_PASSWORD	: sprintf(state, " Newbie"); break;
		case CON_CONFIRM_NEW_PASSWORD	: sprintf(state, " Newbie"); break;
		case CON_GET_NEW_RACE		: sprintf(state, " Newbie"); break;
		case CON_GET_NEW_SEX		: sprintf(state, " Newbie"); break;
		case CON_GET_ALIGNMENT		: sprintf(state, " Newbie"); break;
		case CON_DEFAULT_CHOICE		: sprintf(state, " Newbie"); break;
		case CON_GEN_GROUPS		: sprintf(state, " Newbie"); break;
		case CON_PICK_WEAPON		: sprintf(state, " Newbie"); break;
		case CON_READ_IMOTD		: sprintf(state, "  IMOTD"); break;
		case CON_READ_MOTD		: sprintf(state, "   MOTD"); break;
		case CON_BREAK_CONNECT		: sprintf(state, "  Break"); break;
		case CON_MANUAL			: sprintf(state, " Manual"); break;
		case CON_CONSOLE		: sprintf(state, "Console"); break;
		case CON_WANT_ANSI		: sprintf(state, "  Ansi?"); break;
		case CON_EDITOR			: sprintf(state, " Editor"); break;
		case CON_NAME_HELP		: sprintf(state, "NewChar"); break;
		case CON_TALKING		: sprintf(state, "Talking"); break;
	    }
	    sprintf( buf + strlen(buf), "#1[#n%3d %s#1]#n #7%s#1@#3%s#n\n\r",
		d->descriptor,
		state,
		d->original  ? GET_PC_NAME(d->original)  :
		d->character ? GET_PC_NAME(d->character) : "(none)",
		d->host
		);
	}
    }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if (!IS_GRANTED(ch, CMD_CONTROL) && ch->level != 120)
    {
	send_to_char("You have not been granted this command.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") && !IS_CREATOR(ch) )
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
    sprintf( buf, "$n forces you to '%s'.", argument );
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
		act( buf, ch, NULL, vch, TO_VICT );
    }
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
 
	if (get_trust(ch) < MAX_LEVEL - 2)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}
 
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;
 
	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	    &&   vch->level < LEVEL_HERO)
	    {
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
		act( buf, ch, NULL, vch, TO_VICT );
    }
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"gods"))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
 
	if (get_trust(ch) < MAX_LEVEL - 2)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}
 
	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;
 
	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
	    &&   vch->level >= LEVEL_HERO)
	    {
    if (!IS_SET(ch->flag3, AFF_MOB_PROG_NOSHOW))
    {
		act( buf, ch, NULL, vch, TO_VICT );
    }
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
	{
	    send_to_char("That character is in a private room.\n\r",ch);
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( ch->invis_level)
      {
	  ch->invis_level = 0;
	  act( "$n suddenly appears from nowhere.", ch, NULL, NULL,TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
	  do_visible(ch, "nomessage");	   
      }
      else
      {
	  ch->invis_level = get_trust(ch);
	  act( "$n slowly disappears into thin air.", ch, NULL, NULL,TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
	return;
      }
      else
      {
	  ch->reply = NULL;
	  ch->invis_level = level;
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
	  ch->incog_level = 0;
	  send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
	  ch->incog_level = get_trust(ch);
	  send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Incog level must be between 2 and your level.\n\r",ch);
	return;
      }
      else
      {
	  ch->reply = NULL;
	  ch->incog_level = level;
	  act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (IS_FLAG3(ch, AFF_SAY_LOCK))
    {
	send_to_char("You cannot do anything other than talk.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}

void do_twit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Twit whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if (victim->cur_form == 12)
    {
	send_to_char("You pardon them for being such a twat.\n\r",ch);
	send_to_char("The Gods have forgiven you for being such an idiot.\n\r",victim);
	set_form(victim, 0);
    }
    else
    {
	send_to_char("Ok.  You twit him.  They must be a right twat.\n\r",ch);
	send_to_char("You have been deemed an idiot by the Gods.\n\r",victim);
	set_form(victim, 12);
    }
}

void do_grant(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (  arg1[0] == '\0')
    {
    send_to_char("Syntax:  grant <player> <command group>\n\r",ch);
    send_to_char("         grant <player> list|all|none\n\r",ch);
    send_to_char("Groups:  connections, load, echo, control, channels\n\r",ch);
    send_to_char("         lock, reboot, switch, sites\n\r",ch);
    return;
    }

    if ( arg2[0]=='\0')
    {
	send_to_char("Grant what?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char("They are not logged on.\n\r",ch);
	return;
    }
    
    if (IS_NPC(victim))
    {
	stc("Only on NPC's.\n\r", ch);
	return;
    }

    if ( victim->level < 112 )
    {
	send_to_char("They aren't immortal.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2, "all") )
    {
	SET_GRANT(victim, CMD_CONNECTIONS);
	SET_GRANT(victim, CMD_LOAD);
	SET_GRANT(victim, CMD_ECHO);
	SET_GRANT(victim, CMD_CONTROL);
	SET_GRANT(victim, CMD_CHANNELS);
	SET_GRANT(victim, CMD_LOCK);
	SET_GRANT(victim, CMD_REBOOT);
	SET_GRANT(victim, CMD_SWITCH);
	SET_GRANT(victim, CMD_SITES);
	send_to_char("All grants set.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2, "none") )
    {
	REMOVE_GRANT(victim, CMD_CONNECTIONS);
	REMOVE_GRANT(victim, CMD_LOAD);
	REMOVE_GRANT(victim, CMD_ECHO);
	REMOVE_GRANT(victim, CMD_CONTROL);
	REMOVE_GRANT(victim, CMD_CHANNELS);
	REMOVE_GRANT(victim, CMD_LOCK);
	REMOVE_GRANT(victim, CMD_REBOOT);
	REMOVE_GRANT(victim, CMD_SWITCH);
	REMOVE_GRANT(victim, CMD_SITES);
	send_to_char("All grants removed.\n\r",ch);
	return;
    }

    if ( !str_prefix(arg2, "list") )
    {
	send_to_char("Commands granted:\n\r",ch);
	if (IS_GRANTED(ch, CMD_CONNECTIONS))
		send_to_char("   connections\n\r",ch);
	if (IS_GRANTED(ch, CMD_LOAD))
		send_to_char("   load\n\r",ch);
	if (IS_GRANTED(ch, CMD_ECHO))
		send_to_char("   echo\n\r",ch);
	if (IS_GRANTED(ch, CMD_CONTROL))
		send_to_char("   control\n\r",ch);
	if (IS_GRANTED(ch, CMD_CHANNELS))
		send_to_char("   channels\n\r",ch);
	if (IS_GRANTED(ch, CMD_LOCK))
		send_to_char("   lock\n\r",ch);
	if (IS_GRANTED(ch, CMD_REBOOT))
		send_to_char("   reboot\n\r",ch);
	if (IS_GRANTED(ch, CMD_SWITCH))
		send_to_char("   switch\n\r",ch);
	if (IS_GRANTED(ch, CMD_SITES))
		send_to_char("   sites\n\r",ch);
	return;
    }

    if ( !str_prefix(arg2, "connections") )
    {
	TOGGLE_GRANT(victim, CMD_CONNECTIONS);
	return;
    }

    if ( !str_prefix(arg2, "load") )
    {
	TOGGLE_GRANT(victim, CMD_LOAD);
	return;
    }

    if ( !str_prefix(arg2, "echo") )
    {
	TOGGLE_GRANT(victim, CMD_ECHO);
	return;
    }

    if ( !str_prefix(arg2, "control") )
    {
	TOGGLE_GRANT(victim, CMD_CONTROL);
	return;
    }

    if ( !str_prefix(arg2, "channels") )
    {
	TOGGLE_GRANT(victim, CMD_CHANNELS);
	return;
    }

    if ( !str_prefix(arg2, "lock") )
    {
	TOGGLE_GRANT(victim, CMD_LOCK);
	return;
    }

    if ( !str_prefix(arg2, "reboot") )
    {
	TOGGLE_GRANT(victim, CMD_REBOOT);
	return;
    }

    if ( !str_prefix(arg2, "switch") )
    {
	TOGGLE_GRANT(victim, CMD_SWITCH);
	return;
    }

    if ( !str_prefix(arg2, "sites") )
    {
	TOGGLE_GRANT(victim, CMD_SITES);
	return;
    }

    if ( !str_prefix(arg2, "connections") )
    {
	TOGGLE_GRANT(victim, CMD_CONNECTIONS);
	return;
    }


    /*** None found:  display syntax ***/

    do_grant(ch, "");
}

void do_resetarea(CHAR_DATA *ch, char *argument)
{
    send_to_char("You patiently twiddle your thumbs, waiting for the reset.\n\r",ch);
    send_to_char("Oh fuck this, you MAKE it reset!\n\r",ch);
    area_update();
}

void do_tick(CHAR_DATA *ch, char *argument)
{
    send_to_char("TICK!  Now wasn't that fun for you.\n\r",ch);
    weather_update();
    char_update();
    obj_update();
    room_update();
    area_update();
}

void do_form(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0]=='\0')
    {
	send_to_char("Change your form to resemble what?\n\r",ch);
	return;
    }

    if (!str_cmp(argument, "Mitzrah") ) 
    {
	send_to_char("Hmmm... not a good idea really.\n\r", ch);
	return;
    } 

    sprintf(buf, "%s's form blurs and shifts to resemble %s.",
			ch->name,
			argument);
    act(buf,ch,NULL,NULL,TO_ROOM);
    set_pc_name(ch, argument);
    send_to_char("Your form blurs and shifts to resemble ",ch);
    send_to_char(argument,ch);
    send_to_char(".\n\r",ch);
    set_enter_room_text(ch, "");
}

void do_formobject(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0]=='\0')
    {
	send_to_char("Change your form to which object?\n\r",ch);
	return;
    }

    sprintf(buf, "%s's form blurs and shifts to resemble %s.",
			ch->name,
			argument);
    act(buf,ch,NULL,NULL,TO_ROOM);
    set_pc_name(ch, argument);
    send_to_char("Your form blurs and shifts to resemble ",ch);
    send_to_char(argument,ch);
    send_to_char(".\n\r",ch);
    set_enter_room_text(ch, "");
    SET_BIT(ch->flag3, AFF_CHAR_IS_OBJ);
}

void do_unform(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%s's form blurs and shifts to resemble %s.",
			ch->name,
			GET_PROPER_NAME(ch));
    act(buf,ch,NULL,NULL,TO_ROOM);
    set_pc_name(ch, GET_PROPER_NAME(ch));
    send_to_char("Your form blurs and shifts to resemble ",ch);
    send_to_char(GET_PROPER_NAME(ch),ch);
    send_to_char(".\n\r",ch);
    look_name(ch);
    REMOVE_BIT(ch->flag3, AFF_CHAR_IS_OBJ);
}

void do_smashmask(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    char buf[MAX_STRING_LENGTH];

    if (argument[0]=='\0')
    {
	send_to_char("Smash who's mask?\n\r",ch);
	return;
    }

    if ((vch = get_char_world(ch, argument)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }
    
    if (IS_NPC(vch))
    {
	stc("NPC's cannot mask.\n\r", ch);
	return;
    }

    sprintf(buf,"%s's form blurs and shifts to resemble %s.",
                vch->name,
                vch->pcdata->switchname);
    set_pc_name(vch,vch->pcdata->switchname);
    act(buf,vch,NULL,NULL,TO_ROOM);   
    REMOVE_BIT(vch->flag2,AFF_MASKED);
    send_to_char("Your form blurs and shifts back to normal.\n\r",vch);
    send_to_char("Done master.\n\r",ch);
    change_upkeep(vch, -4);
}

void do_setdisciplines(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    int val;
    int i;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0')
    {
	send_to_char("Set whose disciplines?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    val = atoi(arg3);

    if (val < -2 || val > 10)
    {
	send_to_char("Values between -2 and 10 please.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2, "all"))
    {
	for (i = 0; i < MAX_DISCIPLINES; i++)
	    victim->power[i] = val;

	send_to_char("All disciplines set.\n\r",ch);
	return;
    }

    for (i = 0; i < MAX_DISCIPLINES; i++)
    {
	if (!str_prefix(arg2, discipline[i]))
	{
	    send_to_char("Discipline set.\n\r",ch);
	    victim->power[i] = val;
	    return;
	}
    }
    send_to_char("No such discipline.\n\r",ch);
}

void do_flag(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH],arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    const struct flag_type *flag_table;

    if (!IS_GRANTED(ch, CMD_LOAD) && ch->level!=120)
    {
	send_to_char("You do not have access to this command.\n\r",ch);
	return;
    }

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    argument = one_argument(argument,arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument(argument,word);

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  flag mob  <name> <field> <flags>\n\r",ch);
	send_to_char("  flag char <name> <field> <flags>\n\r",ch);
	send_to_char("  mob  flags: act,aff,off,imm,res,vuln,form,part,new\n\r",ch);
	send_to_char("  char flags: plr,comm,aff,imm,res,vuln,\n\r",ch);
	send_to_char("  +: add flag, -: remove flag, = set equal to\n\r",ch);
	send_to_char("  otherwise flag toggles the flags listed.\n\r",ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	send_to_char("What do you wish to set flags on?\n\r",ch);
	return;
    }

    if (arg3[0] == '\0')
    {
	send_to_char("You need to specify a flag to set.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Which flags do you wish to change?\n\r",ch);
	return;
    }

    if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char"))
    {
	victim = get_char_world(ch,arg2);
	if (victim == NULL)
	{
	    send_to_char("You can't find them.\n\r",ch);
	    return;
	}

        /* select a flag to set */
	if (!str_prefix(arg3,"act"))
	{
	    if (!IS_NPC(victim))
	    {
		send_to_char("Use plr for PCs.\n\r",ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = act_flags;
	}

	else if (!str_prefix(arg3,"plr"))
	{
	    if (IS_NPC(victim))
	    {
		send_to_char("Use act for NPCs.\n\r",ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = plr_flags;
	}

 	else if (!str_prefix(arg3,"aff"))
	{
	    flag = &victim->affected_by;
	    flag_table = affect_flags;
	}

 	else if (!str_prefix(arg3,"new"))
	{
	    flag = &victim->flag2;
	    flag_table = new_flags;
	}

  	else if (!str_prefix(arg3,"immunity"))
	{
	    flag = &victim->imm_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"resist"))
	{
	    flag = &victim->res_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"vuln"))
	{
	    flag = &victim->vuln_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"form"))
	{
	    if (!IS_NPC(victim))
	    {
	 	send_to_char("Form can't be set on PCs.\n\r",ch);
		return;
	    }

	    flag = &victim->form;
	    flag_table = form_flags;
	}

	else if (!str_prefix(arg3,"parts"))
	{
	    if (!IS_NPC(victim))
	    {
		send_to_char("Parts can't be set on PCs.\n\r",ch);
		return;
	    }

	    flag = &victim->parts;
	    flag_table = part_flags;
	}

	else if (!str_prefix(arg3,"comm"))
	{
	    if (IS_NPC(victim))
	    {
		send_to_char("Comm can't be set on NPCs.\n\r",ch);
		return;
	    }

	    flag = &victim->comm;
	    flag_table = comm_flags;
	}

	else
	{
	    send_to_char("That's not an acceptable flag.\n\r",ch);
	    return;
	}

	old = *flag;
	victim->zone = NULL;

	if (type != '=')
	    new = old;

        /* mark the words */
        for (; ;)
        {
	    argument = one_argument(argument,word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word,flag_table);
	    if (pos == 0)
	    {
		send_to_char("That flag doesn't exist!\n\r",ch);
		return;
	    }
	    else
		SET_BIT(marked,pos);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++)
	{
	    if (!flag_table[pos].settable && IS_SET(old,flag_table[pos].bit))
	    {
		SET_BIT(new,flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked,flag_table[pos].bit))
	    {
		switch(type)
		{
		    case '=':
		    case '+':
			SET_BIT(new,flag_table[pos].bit);
			break;
		    case '-':
			REMOVE_BIT(new,flag_table[pos].bit);
			break;
		    default:
			if (IS_SET(new,flag_table[pos].bit))
			    REMOVE_BIT(new,flag_table[pos].bit);
			else
			    SET_BIT(new,flag_table[pos].bit);
		}
	    }
	}
	*flag = new;
	return;
    }
}

