/******************************************************************
 * Trade.c							  *
 * Funky trade like stuff man					  *
 *     - Concept and code (C) 1997 Chris Mellish		  *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "trade.h"


/* trade data - name, research cost, research time, exp per level (accumaltive)
                         (exp)          (ticks)                   , max_level
	       .name     .cost           .time          .exp       .max_level */

const TRADE_DATA trades[MAX_TRADES] =

{
   {"",			-1,		-1,		-1,	0	},
   {"Blacksmithing",	18000,		36,		8000,	0 	},
   {"Riding",		15000,		24,		6000 ,	0 	},
   {"Survival",		14000, 		20,		7000,	10 	},
   {"Fletching",	18000,		40,		8000,	0 	}
};


void do_trades( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int i, found = 0;

    if (IS_NPC(ch)) return;

    if (argument[0] == 0 || !str_prefix(argument, "known"))
    {
	banner2_to_char("TRADES", ch);
	cent_to_char("#7[known]#n [list] [research] [info] [improve]", ch);
	stc("\n\r",ch);
	for (i = 0; i < MAX_TRADES; i++)
	{
	    char expbuf[100];
	    if (ch->trades[i] > -1)
	    {
		found++;
		sprintf(expbuf, "[    N/A    ]");
		if (ch->trades[i] < trades[i].max_level)
		    sprintf(expbuf, "[%6d  exp]",
			((ch->trades[i]+1) * trades[i].exp));
		sprintf(buf, "%-16s : level %d %s", 
			trades[i].name, ch->trades[i], expbuf);
		cent_to_char(buf, ch);
	    }
	}
	if (!found) banner2_to_char("You know no trades",ch);
	else
	{
	    stc("\n\r",ch);
	    sprintf(buf, "You know %d trade%s", found,
		(found == 1) ? "" : "s");
	    banner2_to_char(buf, ch);
	}
	return;
    }

    if (!str_prefix(argument, "list"))
    {
	banner2_to_char("TRADES", ch);
	cent_to_char("[known] #7[list]#n [research] [info] [improve]", ch);
	stc("\n\r",ch);
	for (i = 0; i < MAX_TRADES; i++)
	{
	    if (trades[i].name != NULL && trades[i].name[0] != 0)
	    {
		found++;
		sprintf(buf, "%2d. %20s - %4d exp to learn in %2d ticks. %s",
		    found,
		    trades[i].name,
		    trades[i].cost,
		    trades[i].time,
		    (ch->trades[i] != -1) ? "[KNOWN]" : "       ");
		cent_to_char(buf, ch);
	    }
	}
	sprintf(buf, "%d Trades Found", found);
	banner2_to_char(buf, ch);
	return;
    }

    if (!str_prefix(argument, "information")
	|| (ch->trade_researching != -1 && !str_prefix(argument, "research")))
    {
	banner2_to_char("TRADES", ch);
	cent_to_char("[known] [list] [research] #7[info]#n [improve]", ch);
	stc("\n\r",ch);
	if (ch->trade_researching == -1)
	{
	    cent_to_char("You are not currently researching any trade.", ch);
	}
	else
	{
	    sprintf(buf, "You are currently researching %s.",
		trades[ch->trade_researching].name);
	    cent_to_char(buf, ch);
	    sprintf(buf, "[                    ]");
	    for (i = 0; i < ((20 * (trades[ch->trade_researching].time - ch->trade_time)) / trades[ch->trade_researching].time); i++)
		buf[(i < 20) ? (i + 1) : 15] = '*';
	    cent_to_char(buf, ch);
	}
	stc("\n\r",ch);
	divide6_to_char(ch);
	return;
    }

    argument = one_argument(argument, arg);
    if (!str_prefix(arg, "improve"))
    {
	if (argument[0] == 0)
	{
	    stc("Improve which trade?\n\r", ch);
	    return;
	}

	for (i = 0; i < MAX_TRADES; i++)
	{
	    if (trades[i].name != NULL && trades[i].name[0] != 0
		&& !str_prefix(argument, trades[i].name)
		&& ch->trades[i] > -1)
	    {
		if (ch->trades[i] >= trades[i].max_level)
		{
		    stc("You cannot improve that trade any more.\n\r",ch);
		    return;
		}
		if (ch->currentexp < trades[i].exp * (ch->trades[i]+1))
		{
		    sprintf(buf, "You need %d exp to improve %s.\n\r",
			trades[i].exp * (ch->trades[i]+1),
			trades[i].name);
		    stc(buf, ch);
		    return;
		}
		sprintf(buf, "You improve your skill in %s.\n\r",
		    trades[i].name);
		stc(buf, ch);
		ch->currentexp -= trades[i].exp * (ch->trades[i]+1);
		improve_trade(ch, i);
		return;
	    }
	}
	stc("You dont know of a trade called that.\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "research"))
    {
	if (argument[0] == 0)
	{
	    stc("Research which trade?\n\r",ch);
	    return;
	}

	for (i = 0; i < MAX_TRADES; i++)
	{
	    if (trades[i].name != NULL && trades[i].name[0] != 0
		&& !str_prefix(argument, trades[i].name)
		&& ch->trades[i] == -1)
	    {
		if (ch->currentexp < trades[i].cost)
		{
		    sprintf(buf, "You need %d exp to research %s.\n\r",
			trades[i].cost,
			trades[i].name);
		    stc(buf, ch);
		    return;
		}
		sprintf(buf, "You begin your research into the trade of %s.\n\r",
		    trades[i].name);
		stc(buf, ch);
		ch->trade_researching = i;
		ch->trade_time = trades[i].time;
		ch->currentexp -= trades[i].cost;
		return;
	    }
	}
	stc("There is no such trade you can research.\n\r",ch);
	return;
    }

    stc("Syntax:  trades known\n\r", ch);
    stc("         trades list\n\r", ch);
    stc("         trades information\n\r", ch);
    stc("         trades research <trade>\n\r", ch);
}

void gain_trade( CHAR_DATA *ch )
{
    if (IS_NPC(ch) || ch->trade_researching == -1)
	return;

    stcf(ch, "You have finished your research into %s.\n\r",
	trades[ch->trade_researching].name);

    ch->trades[ch->trade_researching] = 0;
    ch->trade_researching = -1;
    ch->trade_time = -1;
}

void improve_trade(CHAR_DATA *ch, int tr)
{
    if (IS_NPC(ch) || ch->pcdata == NULL)
	return;

    // improve trade
    ch->trades[tr] += 1;

    // give any bonuses

    if (tr == TRADE_SURVIVAL)
    {
	if (ch->trades[tr] == 2)
	    ch->pcdata->learned[gsn_sneak] = UMIN(ch->pcdata->learned[gsn_sneak] + 25, 50);
	if (ch->trades[tr] == 3)
	    ch->pcdata->learned[gsn_hide] = UMIN(ch->pcdata->learned[gsn_sneak] + 25, 50);
	if (ch->trades[tr] == 5)
	    ch->pcdata->learned[gsn_foraging] = UMIN(ch->pcdata->learned[gsn_foraging] + 25, 50);
    }
}

// level 4 survival
void do_findwater(CHAR_DATA *ch, char *argument)
{

}
