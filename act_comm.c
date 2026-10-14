/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "hell.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit     );
DECLARE_DO_FUN(do_dismount );
DECLARE_DO_FUN(do_help     );
DECLARE_DO_FUN(vamp_say    );
DECLARE_DO_FUN(do_wake     );
DECLARE_DO_FUN(do_psioff   );
DECLARE_DO_FUN(do_visible  );

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->level > 111)
   {
	stc("No!\n\r", ch);
	return;
   }

   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N is a dick and deletes $Mself.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);
	    do_quit(ch,"");
	    unlink(strsave);
	    return;
	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible. (like my raincoat)\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}

void do_clear(CHAR_DATA *ch, char *argument)
{
    ansi_cls(ch);
    ansi_goto(ch, 1, 3);
}
	    
void informational( char *argument, int race, int level )
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	    && (race < 0 || d->character->secondaryrace == race ))
	{
	    act_new(argument, d->character, NULL, NULL, TO_CHAR, POS_DEAD);
	}
    }
}

void do_colour(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int i, c = -1;

    if (IS_NPC(ch) || ch->pcdata == NULL)
	return;

    if (argument[0] == 0)
    {
	stc("Current Channel Setup\n\r", ch);	
	stc("======= ======= =====\n\r\n\r", ch);
	stcf(ch, "Prompt for ANSI? : %s\n\r", IS_FLAG5(ch, AFF5_NO_PROMPT_FOR_ANSI) ? "No" : "Yes");
	stcf(ch, "ANSI Colour      : %s\n\r", IS_SET(ch->comm, COMM_ANSI) ? "On" : "Off");
	stcf(ch, "Chat             : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_CHAT], colour_name[ch->pcdata->colours[CHANNEL_CHAT]]);
	stcf(ch, "Music            : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_MUSIC], colour_name[ch->pcdata->colours[CHANNEL_MUSIC]]);
	stcf(ch, "Plead            : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_PLEAD], colour_name[ch->pcdata->colours[CHANNEL_PLEAD]]);
	stcf(ch, "Quote            : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_QUOTE], colour_name[ch->pcdata->colours[CHANNEL_QUOTE]]);
	stcf(ch, "Grats            : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_GRATS], colour_name[ch->pcdata->colours[CHANNEL_GRATS]]);
	stcf(ch, "Tells            : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_TELLS], colour_name[ch->pcdata->colours[CHANNEL_TELLS]]);
	stcf(ch, "Shouts           : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_SHOUTS], colour_name[ch->pcdata->colours[CHANNEL_SHOUTS]]);
	stcf(ch, "God              : #%d%s#n\n\r", ch->pcdata->colours[CHANNEL_GOD], colour_name[ch->pcdata->colours[CHANNEL_GOD]]);
	return;
    }

    if (!str_prefix(argument, "prompt"))
    {
	if (IS_FLAG5(ch, AFF5_NO_PROMPT_FOR_ANSI))
	{
	    REMOVE_BIT(ch->flag5, AFF5_NO_PROMPT_FOR_ANSI);
	    stc("You will now be prompted for ANSI upon connection.\n\r", ch);
	    return;
 	}
	SET_BIT(ch->flag5, AFF5_NO_PROMPT_FOR_ANSI);
	stc("You will now NOT be prompted for ANSI upon connection.\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "off"))
    {
	REMOVE_BIT(ch->comm, COMM_ANSI);
	send_to_char("ANSI Colour no longer displayed.\n\r",ch);
	return;
    }

    if (!str_prefix(argument, "on"))
    {
	SET_BIT(ch->comm, COMM_ANSI);
	send_to_char("Welcome to ANSI #1C#2o#3l#4o#5u#6r#n.\n\r",ch);
	return;
    }

    argument = one_argument(argument, arg);
    if (argument[0] == 0)
    {
	stc("Syntax:  colour <channel> <colour>\n\r", ch);
	stc("         colour [on|off]\n\r", ch);
	return;
    }

    for (i = 0; i < 8; i++)
        if (!str_prefix(argument, colour_name[i])) c = i;

    if (c == -1)
    {
	stc("Unknown colour.\n\r", ch);
	return;
    }

    if (!str_prefix(arg, "chat"))
    {
	stcf(ch, "Chat channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_CHAT] = c;
	return;
    }
    if (!str_prefix(arg, "music"))
    {
	stcf(ch, "Music channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_MUSIC] = c;
	return;
    }
    if (!str_prefix(arg, "plead"))
    {
	stcf(ch, "Plead channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_PLEAD] = c;
	return;
    }
    if (!str_prefix(arg, "quote"))
    {
	stcf(ch, "Quote channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_QUOTE] = c;
	return;
    }
    if (!str_prefix(arg, "grats"))
    {
	stcf(ch, "Grats channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_GRATS] = c;
	return;
    }
    if (!str_prefix(arg, "god"))
    {
	stcf(ch, "God channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_GOD] = c;
	return;
    }
    if (!str_prefix(arg, "shouts"))
    {
	stcf(ch, "Shout channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_SHOUTS] = c;
	return;
    }
    if (!str_prefix(arg, "tells"))
    {
	stcf(ch, "Tell channel set to colour #%d%s#n.\n\r", c, colour_name[c]);
	ch->pcdata->colours[CHANNEL_TELLS] = c;
	return;
    }

    stc("Unknown channel.\n\r", ch);
}

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);
 
    send_to_char("chat           ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("plead          ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("Quote          ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUOTE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("grats          ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATS))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_IMMORTAL(ch))
    {
      send_to_char("god channel    ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
	send_to_char("ON\n\r",ch);
      else
	send_to_char("OFF\n\r",ch);
    }

    send_to_char("shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    send_to_char("tells          ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("ansi colour    ",ch);
    if (IS_SET(ch->comm,COMM_ANSI))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);

    send_to_char("quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
	send_to_char("You are AFK.\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char("You are immune to snooping.\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf,"Your current prompt is: %s\n\r",ch->prompt);
	send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);

}

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear tells again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_AFK))
    {
      send_to_char("AFK mode removed. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   else
   {
     send_to_char("You are now in AFK mode.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
   }
}

void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

void do_gossip( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOSSIP))
      {
	send_to_char("Chat channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      }
      else
      {
	send_to_char("Chat channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOGOSSIP);
      }
    }
    else  /* Chat message sent, turn chat on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
 
	}


      REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
 
      sprintf( buf, "You chat '#%d%s#n'\n\r",
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_CHAT] : 3,
		argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOGOSSIP) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
		name_prefix_to_char(victim, ch);
		sprintf( buf, "$n%s chats '#%d$t#n'",
			 GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_CHAT] : 3);
		act_new( buf, 
		   ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
      }
    }
}

void do_grats( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGRATS))
      {
	send_to_char("Grats channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOGRATS);
      }
      else
      {
	send_to_char("Grats channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOGRATS);
      }
    }
    else  /* grats message sent, turn grats on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
 
	}
 

      REMOVE_BIT(ch->comm,COMM_NOGRATS);
 
      sprintf( buf, "You grats '#%d%s#n'\n\r",
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_GRATS] : 3,
		argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOGRATS) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
		name_prefix_to_char(victim, ch);
		sprintf( buf, "$n%s grats '#%d$t#n'", 
			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_GRATS] : 3);
		act_new( buf, 
		   ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
      }
    }
}

void do_quote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUOTE))
      {
	send_to_char("Quote channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOQUOTE);
      }
      else
      {
	send_to_char("Quote channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOQUOTE);
      }
    }
    else  /* quote message sent, turn quote on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
 
	}
 

      REMOVE_BIT(ch->comm,COMM_NOQUOTE);
 
      sprintf( buf, "You quote '#%d%s#n'\n\r",
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_QUOTE] : 3,
		argument );

      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOQUOTE) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
		name_prefix_to_char(victim, ch);
		sprintf( buf, "$n%s quotes '#%d$t#n'",
 			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_QUOTE] : 3);
		act_new( buf, 
		   ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
      }
    }
}

void do_question( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
	send_to_char("Pleading channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
	send_to_char("Pleading channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* question sent, turn Q/A on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
 

	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
      sprintf( buf, "You plead '#%d%s#n'\n\r",
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_PLEAD] : 3,
		argument );

      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOQUESTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
		name_prefix_to_char(victim, ch);
		sprintf( buf, "$n%s pleads '#%d$t#n'",
			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_PLEAD] : 3);
		act_new( buf, 
		  ch,argument,d->character,TO_VICT,POS_SLEEPING);
	}
      }
    }
}

void do_answer( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
	send_to_char("Pleading channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
	send_to_char("Pleading channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* answer sent, turn Q/A on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
 

	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
      sprintf( buf, "You answer '%s'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOQUESTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
		name_prefix_to_char(victim, ch);
		sprintf( buf, "$n%s answers '$t'", GET_FORM(ch) );
		act_new( buf, 
		  ch,argument,d->character,TO_VICT,POS_SLEEPING);
	}
      }
    }
}

void do_music( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
 
    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMUSIC))
      {
	send_to_char("Music channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      }
      else
      {
	send_to_char("Music channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOMUSIC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}
 
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}
 

	REMOVE_BIT(ch->comm,COMM_NOMUSIC);
 
      sprintf( buf, "You MUSIC: '#%d%s#n'\n\r",
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_MUSIC] : 3,
		argument );

      send_to_char( buf, ch );
      sprintf( buf, "$n%s MUSIC: '%s'",GET_FORM(ch), argument );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOMUSIC) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
		name_prefix_to_char(victim, ch);
		sprintf( buf, "$n%s MUSIC: '#%d$t#n'",
			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_MUSIC] : 3);
		act_new( buf, 
		    ch,argument,d->character,TO_VICT,POS_SLEEPING);
	}
      }
    }
}

/* clan channels */
void do_clantalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if (!is_clan(ch) || clan_table[ch->clan].independent)
    {
	send_to_char("You aren't in a clan.\n\r",ch);
	return;
    }
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCLAN))
      {
	send_to_char("Clan channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOCLAN);
      }
      else
      {
	send_to_char("Clan channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOCLAN);
      }
      return;
    }

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	 send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}

	REMOVE_BIT(ch->comm,COMM_NOCLAN);

      sprintf( buf, "You clan '%s'\n\r", argument );
      send_to_char( buf, ch );
      sprintf( buf, "$n clans '%s'", argument );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     is_same_clan(ch,d->character) &&
	     !IS_SET(d->character->comm,COMM_NOCLAN) &&
	     !IS_SET(d->character->comm,COMM_QUIET) )
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s clans '$t'", GET_FORM(ch) );
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }

    return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Immortal channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Immortal channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      } 
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

    sprintf( buf, "$n%s : #%d%s#n", GET_FORM(ch),
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_GOD] : 3,
		argument );
    act_new("$n : $t",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    sprintf( buf, "$n%s : #%d$t#n", 
			GET_FORM(ch),
			(d->character->pcdata != NULL) ? d->character->pcdata->colours[CHANNEL_GOD] : 3);
	    if (d->character->level > 111)
	    {
		name_prefix_to_char(d->character, ch);
		act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	    }
	}
    }

    return;
}

void do_commune( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to commune?\n\r", ch);
	return;
    }

    sprintf( buf, "$n: %s", argument );
    act_new("$n communes '$t'.",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     is_same_secondaryrace(ch,d->character) )
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s communes '$t'", GET_FORM(ch) );
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }
    return;
}       

void do_howl( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to howl?\n\r", ch);
	return;
    }

    sprintf( buf, "$n: %s", argument );
    act_new("$n howls '$t'.",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&   
	     is_same_secondaryrace(ch,d->character) )
	{
		name_prefix_to_char(d->character, ch);
	    act_new("$n howls '$t'.",ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }
    return;
}   

void do_whisper( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    
    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to whisper?\n\r", ch);
	return;
    }

    sprintf( buf, "$n: %s", argument );
    act_new("$n whispers '$t'.",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     is_same_secondaryrace(ch,d->character) )
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s whispers '$t'", GET_FORM(ch) );
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }
    return;
}

void do_chant( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to sing?\n\r", ch);
	return;
    }

    act_new("$n sings '$t'.",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     is_same_secondaryrace(ch,d->character) )
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s sings '$t'", GET_FORM(ch) );
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }
    return;
}

void do_dream( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to dream of?\n\r", ch);
	return;
    }

    if (ch->position != POS_SLEEPING)
    {
	send_to_char("Stop daydreaming, you have work to do!\n\r", ch);
	return;
    }
    act_new("You dream of .oO($t)",ch,argument,NULL,TO_CHAR,POS_SLEEPING);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING && (
	     d->character->position == POS_SLEEPING
	    || d->character->level > 111 ) )
	{
		name_prefix_to_char(d->character, ch);
	    sprintf( buf, "$n dreams of .oO($t)");
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_SLEEPING);
	}
    }
    if (!str_cmp(argument, "spiders")
	|| !str_cmp(argument, "ghosts")
	|| !str_cmp(argument, "monsters")
	|| !str_cmp(argument, "ginny")
	|| !str_cmp(argument, "trolls"))
    {
	send_to_char("Wooooooh!  What a scary dream.\n\r", ch);
	act("$n wakes up shivering.", ch, NULL, NULL, TO_ROOM);
	do_wake(ch, "");
	return;
    }
    if (!str_cmp(argument, "sex") )
    {
	send_to_char("Oh god!  You suddenly feel all hot and flustered.\n\r",ch);
	act("$n wakes up all hot and flustered.", ch, NULL, NULL, TO_ROOM);
	do_wake(ch, "");
	return;
    }
	
    return;
}

void do_pray( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    
    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to pray?\n\r", ch);
	return;
    }

    sprintf( buf, "$n: %s", argument );
    act_new("$n prays '$t'.",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     is_same_secondaryrace(ch,d->character) )
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s prays '$t'", GET_FORM(ch) );
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }
    return;
}

void do_project( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("What do you wish to incant?\n\r", ch);
	return;
    }

    sprintf( buf, "$n: %s", argument );
    act_new("$n incants '$t'.",ch,argument,NULL,TO_CHAR,POS_DEAD);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     is_same_secondaryrace(ch,d->character) )
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s incants '$t'", GET_FORM(ch) );
	    act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
	}
    }
    return;
}



void do_say( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wch, *wchnext;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    char hog[MAX_STRING_LENGTH];
    char *say, *says, *nl;
    bool adds = FALSE;
    int l, sl;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if (!RTIME_UP(ch->in_room, RTIMER_DISCORD))
    {
	stc("Your words are lost beneath the banging and crashing noises.\n\r", ch);
	act("$n says something, but its too noisy to hear.", ch, NULL, NULL, TO_ROOM);
	return;
    }

    if (ch->visionary != NULL && ch->vision == -2 && ch->visionary->vision < 0)
    {
        sprintf(buf, "%s '%s'\n\r", (ch->alignment < 0) ?
            "A loud voice echoes in your mind" : "The glowing figure says",
            argument);
        send_to_char(buf, ch->visionary);
        sprintf(buf, "%s '%s'\n\r", (ch->alignment < 0) ?
           "Your voice echoes loudly in their mind" : "You say to them",
            argument);
        send_to_char(buf, ch);
	return;
    }

    if (ch->visionary != NULL && ch->vision < 0)
    {
        sprintf(buf, "You reply '%s'\n\r",
            argument);
        send_to_char(buf, ch);
        sprintf(buf, "%s replies '%s'\n\r", GET_NAME(ch),
            argument);
        send_to_char(buf, ch->visionary);
	return;
    }

    say = "say"; says = "says";
    l = strlen(argument) - 1;

    if (argument[l] == '.'
	&& argument[l-1] != '.')
    {
	say = "state";
	says = "states";
    }
    else if (argument[l] == '.'
	&& argument[l-1] == '.'
	&& argument[l-2] == '.')
    {
	say = "mutter";
	says = "mutters";
    }
    else if(argument[l] == '!')
    {
	say = "exclaim";
	says = "exclaims";
    }
    else if (argument[l] =='?')
    {
	say = "ask";
	says = "asks";
    }
    else if (argument[l] == ')'
	    && argument[l-1] == ':')
    {
	say = "smile";
	says = "smiles";
    }
    else if (argument[l] == '('
	    && argument[l-1] == ':')
    {
	say = "frown";
	says = "frowns";
    }
    if (ch->rage < 50 && ch->rage > 0)
    {
	say = "snarl";
	says = "snarls";
    }
    else if (ch->rage > 49)
    {
	say = "roar";
	says = "roars";
    }

    if (ch->cur_form == FRM_WOLF)
    {
	say = "growl";
	says = "growls";
    }

    if (ch->cur_form == FRM_FROG)
    {
	say = "croak";
	says = "croaks";
    }

    if (ch->cur_form == FRM_HAWK)
    {
	say = "screech";
	says = "screeches";
    }

    if (ch->cur_form == FRM_BAT)
    {
	say = "chirp";
	says = "chirps";
    }

    if (form_data[ch->cur_form].say_word[0] != '\0')
    {
	say = form_data[ch->cur_form].say_word;
	says = form_data[ch->cur_form].say_word;
	adds = TRUE;
    }

    if (IS_NPC(ch))
	ch->curlang = 1;

    if (ch->curlang == 3) ch->curlang = 1;

    switch( ch->curlang )
    {
	case 1 : sl = LANG_COMMON; nl = "common"; break;
	case 2 : sl = LANG_ELVISH; nl = "elvish"; break;
	case 3 : sl = LANG_COMMON; nl = "common"; break;
	case 4 : sl = LANG_GNOMISH; nl = "gnome"; break;
	case 5 : sl = LANG_DEMONIC; nl = "the demonic tongue"; break;
	default: sl = LANG_COMMON; nl = "common"; break;
    }

    if (IS_FLAG3(ch, AFF_BABBLE) ) conv_language(argument, buf3, LANG_BABBLE, ch);
    else if (sl != LANG_COMMON) conv_language(argument, buf3, ch->curlang, ch);
    else sprintf(buf3, argument);

    sprintf(buf2, " %s%s in %s '%s'\n\r",
	says,
	adds ? "s" : "",
	nl,
	buf3);

    sprintf(name, "%s%s",
	    GET_NAME(ch),
	    GET_FORM(ch));

    name[0] = UPPER(name[0]);

    sprintf(buf, "You %s in %s '%s'\n\r", say, nl,
	(sl == LANG_COMMON) ? buf3 : argument);
    send_to_char(buf, ch);

    if (ch->race == RACE_GNOME
	&& ch->secondaryrace != RACE_DAEMON)
	smash_space(argument);

    sprintf(buf, " %s%s in %s '%s'\n\r",
	says,
	adds ? "s" : "",
	nl,
	argument);

    for (wch = ch->in_room->people; wch != NULL; wch = wchnext)
    {
	wchnext = wch->next_in_room;
	if (wch->position <= POS_SLEEPING) continue;
	if (wch == ch) continue;

	name_prefix_to_char(wch, ch);
	if (is_item(ch) && ch->sw_obj->in_obj != NULL)
   	{
	    sprintf(hog, "A muffled voice");
	}
	else   
	{
	    sprintf(hog, "%s", person(ch, wch) );
	    hog[0] = UPPER(hog[0]);
	}
	stc(hog,wch);

 	if (!can_speak(wch, sl))
	{
	    send_to_char(buf2, wch);
	    check_trigger(ch, wch, TRIG_SAY, buf3);
	}
	else
	{
	    send_to_char(buf, wch);
	    check_trigger(ch, wch, TRIG_SAY, argument);
	}
    }

    return;

/*    vamp_say( ch, argument); */
}


void do_think( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if ( argument[0] == '\0' )
    {
	send_to_char( "What are you thinking?\n\r", ch );
	return;
    }
    sprintf( buf, "$n%s thinks .oO($T)", GET_FORM(ch) );
    act_new( buf, ch, NULL, argument, TO_ROOM,POS_RESTING);
    act( "You think .oO($T)", ch, NULL, argument, TO_CHAR );
    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0' )
    {
	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
	{
	    send_to_char("You can hear shouts again.\n\r",ch);
	    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
	}
	else
	{
	    send_to_char("You will no longer hear shouts.\n\r",ch);
	    SET_BIT(ch->comm,COMM_SHOUTSOFF);
	}
	return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't shout.\n\r", ch );
	return;
    }
 

    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

    WAIT_STATE( ch, 12 );

    sprintf(buf, "You shout '#%d%s#n'",
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_SHOUTS] : 3,
		argument );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_SHOUTSOFF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
		name_prefix_to_char(d->character, ch);
		sprintf( buf, "$n%s shouts '#%d$t#n'", 
			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_SHOUTS] : 3);
	    act(buf,ch,argument,d->character,TO_VICT);
	}
    }

    return;
}



void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && !IS_FLAG5(victim, AFF5_WHO_LIST) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"%s %s tells you '%s'\n\r",PERS(ch,victim),GET_FORM(ch),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"%s %s tells you '%s'\n\r",PERS(ch,victim),GET_FORM(ch),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }
    sprintf( buf, "You tell $N%s '#%d$t#n'", GET_FORM(victim),
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_TELLS] : 3);

    act( buf, ch, argument, victim, TO_CHAR );

    name_prefix_to_char(victim, ch);
    sprintf( buf, "$n%s tells you '#%d$t#n'", 
			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_TELLS] : 3);
    act_new( buf , ch , argument , victim , TO_VICT,POS_DEAD);
    victim->reply       = ch;
    return;
}

void do_noreply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;

    for (vch = char_list; vch != NULL; vch = vch->next )
	if (vch->reply == ch) vch->reply = NULL;

    stc("Sure.\n\r", ch);
}

void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    if (ch->visionary != NULL && ch->vision == -2 && ch->visionary->vision < 0)
    {
        sprintf(buf, "%s '%s'\n\r", (ch->alignment < 0) ?
            "A loud voice echoes in your mind" : "The glowing figure says",
            argument);
        send_to_char(buf, ch->visionary);
        sprintf(buf, "%s '%s'\n\r", (ch->alignment < 0) ?
           "Your voice echoes loudly in their mind" : "You say to them",
            argument);
        send_to_char(buf, ch);
	return;
    }

    if (ch->visionary != NULL && ch->vision < 0)
    {
        sprintf(buf, "You reply '%s'\n\r",
            argument);
        send_to_char(buf, ch);
        sprintf(buf, "%s replies '%s'\n\r", GET_NAME(ch),
            argument);
        send_to_char(buf, ch->visionary);
	return;
    }

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"%s %s tells you '%s'\n\r",PERS(ch,victim),GET_FORM(ch),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
	act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act_new("$E is AFK, and not receiving tells.",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
	    return;
	}
 
	act_new("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR,POS_DEAD);
	sprintf(buf,"%s %s tells you '%s'\n\r",PERS(ch,victim),GET_FORM(ch),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }
    sprintf( buf, "You tell $N%s '#%d$t#n'", GET_FORM(victim),
		(ch->pcdata != NULL) ? ch->pcdata->colours[CHANNEL_TELLS] : 3);

    act_new( buf,ch,argument,victim,TO_CHAR,POS_DEAD);
    name_prefix_to_char(victim, ch);
    sprintf( buf, "$n%s tells you '#%d$t#n'", 
			GET_FORM(ch),
			(victim->pcdata != NULL) ? victim->pcdata->colours[CHANNEL_TELLS] : 3);
    act_new( buf ,ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply       = ch;

    return;
}


void do_yell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char buf[MAX_INPUT_LENGTH];

  /*  if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't yell.\n\r", ch );
	return;
    }*/
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }


    act("You yell '$t'",ch,argument,NULL,TO_CHAR);
    for ( vch = char_list; vch != NULL; vch = vch->next )
    {
	if ( vch != ch
	&&   vch->in_room != NULL
	&&   vch->in_room->area == ch->in_room->area)
	{
	    name_prefix_to_char(vch, ch);
	    sprintf( buf, "$n%s yells '$t'", GET_FORM(ch) );
	    act(buf,ch,argument,vch,TO_VICT);
	}
    }
    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
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
 
    act( "$n $T", ch, NULL, argument, TO_ROOM );
    act( "$n $T", ch, NULL, argument, TO_CHAR );
    return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
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
 
    act( "$n $t", ch, argument, NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
	    act("$N $t",vch,argument,ch,TO_CHAR);
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

	act("$N $t",vch,temp,ch,TO_CHAR);
    }
	
    return;
}

void do_bug( CHAR_DATA *ch, char *argument )
{
    if (argument[0] == '\0')
    {
	send_to_char("You must supply an argument.\n\r", ch);
	return;
    }
	
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    if (argument[0] == '\0')
    {
	send_to_char("You must supply an argument.\n\r", ch);
	return;
    }
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id,sn;

    if ( IS_NPC(ch) )
	return;

    crash("do_quit place 1");

    if (!TIME_UP(ch, TIMER_FIGHT_LAG))
    {
	send_to_char("You cannot quit so soon after a player fight.\n\r",ch);
	return;
    }

    if (ch->visionary != NULL)
    {
	send_to_char("It's rude to quit while you are being spoken to.\n\r",ch);
	return;
    }

    if (ch->riding != NULL)
    {
	do_dismount(ch, "");
	return;
    }

    if (ch->riden_by != NULL)
    {
	do_dismount(ch->riden_by, "");
	return;
    }

    crash("do dismount in quit?");

    if (ch->riding != NULL) ch->riding->riden_by = NULL;
    ch->riding = NULL;
    if (ch->riden_by != NULL) ch->riden_by->riding = NULL;
    ch->riden_by = NULL;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }
    crash("do_quit place 2");

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
	if (ch->psi_upkeep[sn].dur != -1)
	{
	    stc("You cut your psionic links.\n\r", ch);
	    do_psioff(ch, "all");
	    break;
	}
    }

    crash("do_quit place 3");

    if (ch->shadow_victim != NULL)
    {
	ch->shadow_victim->shadow_master = NULL;
	ch->shadow_victim = NULL;
	REMOVE_BIT(ch->flag4, AFF_SHADOW_HIDE);
    }

    if (ch->shadow_master != NULL)
    {
	ch->shadow_master->shadow_victim = NULL;
	REMOVE_BIT(ch->shadow_master->flag4, AFF_SHADOW_HIDE);
	ch->shadow_master = NULL;
    }

    crash("do_quit place 4");

    /* If they are carrying any objects with people in....
	Extract the character from the object */

    obj_exorcise(ch);

    do_help(ch, "@GAMEOVER");

    crash("do_quit place 4 and a bit");

    send_to_char( "The powers acknowledge your cowardice, and await your return.\n\r",ch);
    send_to_char( "\n\r",ch);
    act( "$n has quit and left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", GET_PROPER_NAME(ch) );

    crash("do_quit place 5");

    log_string( log_buf );
    wiznet("$N buggers off.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));
    crash("do_quit place 6");


    /*
     * After extract_char the ch is no longer valid!
     */
    if (ch->registerred == 1) save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    crash("do_quit place 7");
    if ( d != NULL )
	close_socket( d );

    crash("do_quit place 8");
    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    /* whois */
	    logf("Possible bug: Toasting an evil cheating bastard: %s.", tch->name);
/*	    extract_char(tch,TRUE);
	    close_socket(d); */
	} 
    }

    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if (ch->registerred == 0)
    {
	stc("#3>#1*#3>#1*#3>#n You #7MUST#n register before you can save #3<#1*#3<#1*#3<#n\n\r",ch);
	return;
    }

    check_player_table(ch);
    save_char_obj( ch );
    send_to_char("Your character has been saved.\n\r", ch);

    if (is_clan_leader(ch))
    {
	save_clan_leaders();
	stc("Your clan details have also been saved.\n\r", ch);
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
}



void do_shadow_hide( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' && IS_FLAG4(ch, AFF_SHADOW_HIDE ))
    {
	act("You step out of $N's shadow.", ch, NULL, ch->shadow_victim, TO_CHAR);
	REMOVE_BIT(ch->flag4, AFF_SHADOW_HIDE);
	ch->shadow_victim->shadow_master = NULL;
	ch->shadow_victim = NULL;
	do_visible(ch, "nomessage");
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You can't step into your own shadow.\n\r", ch);
	return;
    }

    act("You step into $N's shadow.", ch, NULL, victim, TO_CHAR);
    ch->shadow_victim = victim;
    victim->shadow_master = ch;
    SET_BIT(ch->flag4, AFF_SHADOW_HIDE);
    act("$n steps into the shadows and is gone.", ch, NULL, NULL, TO_ROOM);
    return;
}

void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if (IS_FLAG4(ch, AFF_SHADOW_HIDE))
    {
	send_to_char("Not while you are hiding in a shadow.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch->master )
    {
	act("But you are already following $N.", ch, NULL, victim, TO_CHAR);
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
	     ch,NULL,victim, TO_CHAR);
	return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    crash("nuke pets");

    if ((pet = ch->pet) != NULL)
    {
	stop_follower(pet);
	if (pet->in_room != NULL)
	    act("$N is dragged off screaming, into the darkness...",ch,NULL,pet,TO_NOTVICT);
	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
	if (ch->master->pet == ch)
	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
	send_to_char("You are a very bad person for typing that.\n\r",ch);
	return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "Are you blind?  They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"%-16s %4d/%4d hp %4d/%4d mv %5ld xp\n\r",
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->move,  gch->max_move,
		    gch->currentexp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }
    
    if ((ch->secondaryrace == RACE_LUPINE && victim->secondaryrace != RACE_LUPINE)
	|| (ch->secondaryrace == RACE_DAEMON && victim->secondaryrace == RACE_ELADRIN)
	|| (ch->secondaryrace == RACE_ELADRIN && victim->secondaryrace != RACE_ELADRIN))
    {
	stc("Group with them!?  Out of the question.\n\r",ch);
	return;
    }

    if ((victim->secondaryrace == RACE_LUPINE && ch->secondaryrace != RACE_LUPINE)
	|| (victim->secondaryrace == RACE_DAEMON && ch->secondaryrace == RACE_ELADRIN)
	|| (victim->secondaryrace == RACE_ELADRIN && ch->secondaryrace != RACE_ELADRIN))
    {
	stc("They refuse to group with you!  How rude.\n\r",ch);
	return;
    }

    if (ch->secondaryrace == RACE_DRAGON)
    {
	if ((victim->alignment > 300 && ch->alignment < 300)
		|| (victim->alignment < -300 && ch->alignment > -300))
	{
	    stc("You would never group with such scum.\n\r", ch);
	    return;
	}
    }
    if (victim->secondaryrace == RACE_DRAGON)
    {
	if ((ch->alignment > 300 && victim->alignment < 300)
		|| (ch->alignment < -300 && victim->alignment > -300))
	{
	    stc("They refuse to group with you!\n\r", ch);
	    return;
	}
    }



    if (IS_AFFECTED(victim,AFF_CHARM))
    {
	send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
	act_new("You like your master too much to leave $m!",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act_new("$n removes $N from $s group.",
	    ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	act_new("$n removes you from $s group.",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
	act_new("You remove $N from your group.",
	    ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }

    victim->leader = ch;
    act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("You join $n's group.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if ( amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->silver  -= amount_silver;
    ch->silver  += share_silver + extra_silver;
    ch->gold    -= amount_gold;
    ch->gold    += share_gold + extra_gold;

    if (share_silver > 0)
    {
	sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	     amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
    }
    else
    {
	sprintf(buf,
"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
	 amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share_gold;
	    gch->silver += share_silver;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    act_new("$n tells the group '$t'",
		ch,argument,gch,TO_VICT,POS_SLEEPING);
    }

    act_new("You tell the group '$t'", ch, argument, NULL, TO_CHAR,POS_SLEEPING);

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}


