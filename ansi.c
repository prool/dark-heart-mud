/******************************************************************
 * Miscellanious Ansi functions - (C) 1997 Chris Mellish	  *
 * codes taken from help ansi.sys in MS-DOS			  *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"

void ansi_goto(CHAR_DATA *ch, int x, int y)
{
    char buf[200];
    char digit[200];
    int i = 0;
    /* goto co-ordinate x, y where (0,0) = top left */

    buf[i++] = 27;
    buf[i++] = '[';

    if (x < 0 || x > 79 || y < 0)
    {
	/* home cursor - (0,0) */
	buf[i++] = 'H';
	buf[i++] = '\0';
	stc(buf, ch);
	return;
    }

    sprintf(digit, "%d;%dH", y, x);

    buf[i++] = '\0';
    strcat(buf, digit);
    stc(buf, ch);
}

void ansi_cls(CHAR_DATA *ch)
{
    char esc[10];
    esc[0] = 27;
    esc[1] = '[';
    esc[2] = '2';
    esc[3] = 'J';
    esc[4] = '\0';
    stc(esc, ch);
}

void ansi_goto_desc(DESCRIPTOR_DATA *d, int x, int y)
{
    char buf[200];
    char digit[200];
    int i = 0;
    /* goto co-ordinate x, y where (0,0) = top left */

    buf[i++] = 27;
    buf[i++] = '[';

    if (x < 0 || x > 79 || y < 0)
    {
	/* home cursor - (0,0) */
	buf[i++] = 'H';
	buf[i++] = '\0';
	write_to_descriptor(d->descriptor, buf, 0);
	return;
    }

    sprintf(digit, "%d;%dH", y, x);

    buf[i++] = '\0';
    strcat(buf, digit);
    write_to_descriptor(d->descriptor, buf, 0);
}

void ansi_cls_desc(DESCRIPTOR_DATA *d)
{
    char esc[10];
    esc[0] = 27;
    esc[1] = '[';
    esc[2] = '2';
    esc[3] = 'J';
    esc[4] = '\0';
    write_to_descriptor(d->descriptor, esc, 0);
}

void ansi_up(CHAR_DATA *ch, int n)
{
    char buf[200];
    char esc[10];
    esc[0] = 27;
    esc[1] = '[';
    esc[2] = '\0';

    sprintf(buf, "%s%dA",esc,n);
    stc(buf, ch);
}

void ansi_down(CHAR_DATA *ch, int n)
{
    char buf[200];
    char esc[10];
    esc[0] = 27;
    esc[1] = '[';
    esc[2] = '\0';

    sprintf(buf, "%s%dB",esc,n);
    stc(buf, ch);
}

void ansi_left(CHAR_DATA *ch, int n)
{
    char buf[200];
    char esc[10];
    esc[0] = 27;
    esc[1] = '[';
    esc[2] = '\0';

    sprintf(buf, "%s%dD",esc,n);
    stc(buf, ch);
}

void ansi_right(CHAR_DATA *ch, int n)
{
    char buf[200];
    char esc[10];
    esc[0] = 27;
    esc[1] = '[';
    esc[2] = '\0';

    sprintf(buf, "%s%dC",esc,n);
    stc(buf, ch);
}

void send_title( DESCRIPTOR_DATA *d )
{
    HELP_DATA *help;
    int y = 0, len, x, xd;
    char outbuf[120];
    char tempbuf[120], *tb;

    if (d->value == 0) ansi_cls_desc( d );

    d->value += 10;

    if (d->value >= 80)
    {
	extern char *help_greeting;
	char numbuf[100], num2buf[100], num3buf[100];
	int i, j;
	d->value = -1;
	ansi_goto_desc( d, 1, 1 );
#if 0 // 1 - standard mode, 0 - prool mode
	write_to_buffer( d, help_greeting, 0);
#else
	write_to_buffer( d, "\r\nBy Prool\r\nName? \r\n", 0);
#endif
	hits += 1;
 	if (hits > 99999) { hits -= 99999; highhits += 1; }
	save_hits();
	sprintf(num2buf, "%d", highhits);
	sprintf(num3buf, "%d", hits);
	sprintf(numbuf, "000000000");
	i = 5 - strlen(num2buf);
	for (j = 0; num2buf[j] != 0; j++)
	    numbuf[i++] = num2buf[j];
	i = 9 - strlen(num3buf);
	for (j = 0; num3buf[j] != 0; j++)
	    numbuf[i++] = num3buf[j];
	numbuf[i] = 0;
	sprintf(outbuf, "     ---===> %s Connections since 18:00 Sat 2nd 1997 <===---\n\r\n\rState thy name?  ", numbuf);
	write_to_buffer( d, outbuf, strlen(outbuf));
	d->incomm[0] = 0;
	return;
    }

    if (help_first == NULL) return;

    for (help = help_first; str_cmp(help->keyword, "Dark Heart"); help = help->next)
	if (help->next == NULL) return;

    if (help->text == NULL) return;

    /* help = help Dark Heart.   Fill text buffer with characters */

    tb = help->text;
    tb = split_misc2(tb, tempbuf, '\n');

    ansi_goto_desc( d, 1, 2 );
    ansi_red_desc( d );

    while (tempbuf[0] != '\0')
    {
	len = strlen(tempbuf);
	for (x = 0; x < 119; x++)
	    outbuf[x] = ' ';
	outbuf[119] = 0;
	// tempbuf has the line
	if (y % 2 == 0)
	{
		// scroll from right
	    if (len < d->value)
	    {
		x = 80 - d->value;
		for (xd = 0; tempbuf[xd] != 0; xd++)
		{
		    outbuf[x++] = tempbuf[xd];
		}
		outbuf[79] = 0;
	    }
	    else
	    {
		x = 80 - d->value;
		for (xd = 0; x < 80; xd++)
		{
		    outbuf[x++] = tempbuf[xd];
		}
		outbuf[79] = 0;
	    }
	    
	}
	else
	{
		// scroll from left
	    if (len < d->value)
	    {
		x = d->value - len;
		for (xd = 0; tempbuf[xd] != 0; xd++)
		{
		    outbuf[x++] = tempbuf[xd];
		}
		outbuf[x] = 0;
	    }
	    else
	    {
		x = 0;
		for (xd = len - d->value; tempbuf[xd] != 0; xd++)
		{
		    outbuf[x++] = tempbuf[xd];
		}
		outbuf[x] = 0;
	    }
	}
	write_to_descriptor(d->descriptor, outbuf, 0);
	write_to_descriptor(d->descriptor, "\n\r", 2);
    	tb = split_misc2(tb, tempbuf, '\n');
	y += 1;
    }

}

void ansi_split_screen( CHAR_DATA *ch )
{
   char buf[200];

   sprintf(buf, "%c[%d;%dr", 27, 1, 20);
   stc(buf, ch);
   SET_BIT(ch->flag4, AFF_SCREEN_SPLIT);
   ansi_cls( ch );
   ansi_cls( ch );
   ansi_goto( ch, 1, 22 );
   ansi_goto( ch, 1, 22 );
   ansi_save_cursor( ch );
   ansi_save_cursor( ch );
   ansi_goto( ch, 1, 19 );
   ansi_goto( ch, 1, 19 );
}

void ansi_reset( CHAR_DATA *ch )
{
    char buf[100];

    sprintf(buf, "%cc", 27);
    stc(buf, ch);
    ansi_cls( ch );
    REMOVE_BIT(ch->flag4, AFF_SCREEN_SPLIT);
}

void ansi_save_cursor( CHAR_DATA *ch )
{
    char buf[100];
    sprintf(buf, "%c7", 27);
    stc(buf, ch);
    stc(buf, ch);
    stc(buf, ch);
}

void ansi_restore_cursor( CHAR_DATA *ch )
{
    char buf[100];
    sprintf(buf, "%c8", 27);
    stc(buf, ch);
    stc(buf, ch);
    stc(buf, ch);
}

void ansi_red_desc( DESCRIPTOR_DATA *d )
{
    char buf[100];

    sprintf(buf, "%c[1;31m", 27);
    write_to_descriptor( d->descriptor, buf, 0 );
}
