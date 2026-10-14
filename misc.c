/***************************************************************************
 * MISC.C  We changed all these nasty commands.  They are cool now	   *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "hell.h"


/* New Aliasing by Chris - cause the old one didn't like multi commands, 
	and was a bit poo anyway

   A bit poo?  It was very poo indeed - Adam */

char *const distance[4]=
{
    "right here.", "nearby to the %s.", "not far %s.", "off in the distance %s." 
};

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               sh_int depth, sh_int door));

void substitute_alias(DESCRIPTOR_DATA *d, char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],prefix[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
    char *endbuf;
    char *point;
    int alias;
    bool changed = FALSE;

    if (argument[0] == '\0' || d->connected != CON_PLAYING) return;

    ch = d->original ? d->original : d->character;

    if (ch->prefix[0] != '\0' && str_prefix("prefix", argument))
    {
	if (strlen(ch->prefix) + strlen(argument) > MAX_INPUT_LENGTH)
	    send_to_char("Line to long, prefix not processed.\r\n",ch);
	else
	{
	    sprintf(prefix,"%s %s",ch->prefix,argument);
	    argument = prefix;
	    changed = TRUE;
	}
    }

    if (IS_NPC(ch)
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument))
	return;

    strcpy(buf,argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    continue;

	if (!str_prefix(ch->pcdata->alias[alias],argument))
	{
	    point = one_argument(argument,name);

	    if (!strcmp(ch->pcdata->alias[alias],name))
		sprintf(buf,"%s %s", ch->pcdata->alias_sub[alias], point);

	    if (strlen(buf) > MAX_INPUT_LENGTH)
	    {
		send_to_char("Alias substitution too long. Truncated.\r\n",ch);
		buf[MAX_INPUT_LENGTH -1] = '\0';
	    }
	    changed = TRUE;
	    break;
	}
    }
    if (changed)
    {
	int i, j;
	char tempbuf[MAX_INPUT_LENGTH];

   	endbuf = split_semicolon(d->incomm, name);
	tempbuf[0] = '\0';
	strcpy(tempbuf, endbuf);
	i = 0;
	for (j = 0; buf[j] != '\0'; j++)
	    d->incomm[i++] = buf[j];
	if (tempbuf[0] == '\0')
	{
	    d->incomm[i] = '\0';
	}
	else
	{
	    d->incomm[i] = ';';
	    i++;
	    for (j = 0; tempbuf[j] != '\0'; j++)
		d->incomm[i++] = tempbuf[j];
	    d->incomm[i] = '\0';
	}
    }
}

void do_alia(CHAR_DATA *ch, char *argument)
{
    send_to_char("I'm sorry, alias must be entered in full.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;

    smash_tilde( argument );

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);
 

    if (arg[0] == '\0')
    {

	if (rch->pcdata->alias[0] == NULL)
	{
	    send_to_char("You have no aliases defined.\n\r",ch);
	    return;
	}
	send_to_char("Your current aliases are:\n\r",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    sprintf(buf,"    %s:  %s\n\r",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!str_prefix("una",arg) || !str_cmp("alias",arg))
    {
	send_to_char("Sorry, that word is reserved.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos]))
	    {
		sprintf(buf,"%s aliases to '%s'.\n\r",rch->pcdata->alias[pos],
			rch->pcdata->alias_sub[pos]);
		send_to_char(buf,ch);
		return;
	    }
	}

	send_to_char("That alias is not defined.\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
    {
	send_to_char("That shall not be done!\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
	    sprintf(buf,"%s is now realiased to '%s'.\n\r",arg,argument);
	    send_to_char(buf,ch);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	send_to_char("Sorry, you have reached the alias limit.\n\r",ch);
	return;
     }

     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     sprintf(buf,"%s is now aliased to '%s'.\n\r",arg,argument);
     send_to_char(buf,ch);
}


void do_unalias(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);

    if (arg == '\0')
    {
	send_to_char("Unalias what?\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    send_to_char("Alias removed.\n\r",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char("No alias of that name to remove.\n\r",ch);
}
void do_scan(CHAR_DATA *ch, char *argument)
{
   char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *scan_room;
   EXIT_DATA *pExit;
   sh_int door, depth;
   int alt;

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Looking around you see:\n\r", ch);
                scan_list(ch->in_room, ch, 0, -1);

      for (door=0;door<4;door++)
      {
         if ((pExit = ch->in_room->exit[door]) != NULL
		&& can_see_room(ch, ch->in_room->exit[door]->u1.to_room) )
            scan_list(pExit->u1.to_room, ch, 1, door);
      }
      return;
   }
   else if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
   else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))  door = 1;
   else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
   else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))  door = 3;
   else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" ))   door = 4;
   else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))  door = 5;
   else { send_to_char("Which way do you want to scan?\n\r", ch); return; }

   act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
   act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
   sprintf(buf, "Looking %s you see:\n\r", dir_name[door]);
 
   scan_room = ch->in_room;

   /* Scan up and scan down - Jander */
   if (door == 4)
   {
	for (alt = ch->altitude ; alt <= 10 ; alt++)
	{
	}
	return;
   }


   if (door == 5)
   {
	for (alt = ch->altitude ; alt >= 0 ; alt--)
	{
	}
	return;
   }

   for (depth = 1; depth < 4; depth++)
   {
      if ((pExit = scan_room->exit[door]) != NULL)
      {
         scan_room = pExit->u1.to_room;
         scan_list(pExit->u1.to_room, ch, depth, door);
      }
   }
   return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth,
               sh_int door)
{
   CHAR_DATA *rch;

   if (scan_room == NULL) return;
   for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
   {
      if (rch->altitude != ch->altitude);
      if (rch == ch) continue;
      if (!IS_NPC(rch) && rch->invis_level > get_trust(ch)) continue;
      if (can_see(ch, rch)) scan_char(rch, ch, depth, door);
   }
   return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
   extern char *const distance[];
   char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

   buf[0] = '\0';

   if (weird_weather_type != WEIRD_WEATHER_FOG)
	strcat(buf, PERS(victim, ch));
   else strcat(buf, "A misty figure");

   strcat(buf, ", ");
   sprintf(buf2, distance[depth], dir_name[door]);
   strcat(buf, buf2);
   strcat(buf, "\n\r");

   send_to_char(buf, ch);
   return;
}
