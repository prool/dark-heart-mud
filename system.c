/******************************************************************
 * Code, functions, concept and general sexiness		  *
 * (C) 1997 Chris Mellish - for BTL 2.6				  *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "hell.h"
#include "telnet.h"
#include "recycle.h"


int y_pos args( ( CHAR_DATA *ch ) );
int x_pos args( ( CHAR_DATA *ch ) );
int cy_pos args( ( CHAR_DATA *ch ) );
int cx_pos args( ( CHAR_DATA *ch ) );

void insert_nl 		args( (CHAR_DATA *ch, int y, int x) );
void insert_text	args( (CHAR_DATA *ch, int y, int x, char c) );
void delete_text	args( (CHAR_DATA *ch, int y, int x) );
void delete_line	args( (CHAR_DATA *ch, int y) );
void redraw_charac	args( (CHAR_DATA *ch, int x, int y, int cx, int cy) );
void redraw_line	args( (CHAR_DATA *ch, int y, int cy) );


extern int port,control; /* db.c */                                          
DECLARE_DO_FUN( do_look );

void do_version(CHAR_DATA *ch, char *argument)
{
    stc(MUD_VERSION, ch);
}

int y_pos(CHAR_DATA *ch)
{
    if (ch->pcdata == NULL)
	return -1;

    return ch->pcdata->scry + ch->pcdata->csry;
}

int cy_pos(CHAR_DATA *ch)
{
    if (ch->pcdata == NULL)
	return -1;

    return ch->pcdata->csry;
}

int x_pos(CHAR_DATA *ch)
{
    if (ch->pcdata == NULL)
	return -1;

    return ch->pcdata->csrx;
}

int cx_pos(CHAR_DATA *ch)
{
    if (ch->pcdata == NULL)
	return -1;

    if (strlen(ch->pcdata->edit[y_pos(ch)]) < x_pos(ch))                    
            ch->pcdata->csrrx = strlen(ch->pcdata->edit[y_pos(ch)]);
        else                                          
            ch->pcdata->csrrx = x_pos(ch);         


    return ch->pcdata->csrrx;
}

void editor( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;
    char buf[200];
    int i;

    if (d->character == NULL) return;

    ch = d->character;
    if (IS_NPC(ch) || ch->pcdata == NULL) return;

    for (i = 0; d->incomm[i] != '\0'; i++)
    {
	if (IS_SET(ch->pcdata->edit_flags, EF_FIX))
	{
	    if (d->incomm[i] < 33 || d->incomm[i] == '[')
	        continue;
	    REMOVE_BIT(ch->pcdata->edit_flags, EF_FIX);
	    continue;
	}
	if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	{
	    if (strlen(&(ch->pcdata->edit[y_pos(ch)][40])) < x_pos(ch))
	    	ch->pcdata->csrrx = strlen(&(ch->pcdata->edit[y_pos(ch)][40]));
	    else
	    	ch->pcdata->csrrx = x_pos(ch);
	}
	else
	{
	    if (strlen(ch->pcdata->edit[y_pos(ch)]) < x_pos(ch))
	    	ch->pcdata->csrrx = strlen(ch->pcdata->edit[y_pos(ch)]);
	    else
	    	ch->pcdata->csrrx = x_pos(ch);
	}

	if (ch->pcdata->edit_con == EDIT_INITIALISING)
	{
	    if (d->incomm[i] == 27) ch->pcdata->edit_con = EDIT_EDITING;
	    continue;
	}
	if (ch->pcdata->edit_con == EDIT_TEST)
	{
	    if (d->incomm[i] == 3) 
	    {
		ch->pcdata->edit_con = EDIT_EDITING;
		redraw_editor( d );
	    }
	    else
	    {
		sprintf(buf, "%c(%d)  ", 
		    (d->incomm[i] == 27) ? '*' : (d->incomm[i] > 31) ? d->incomm[i] : ' ', 
		    (int) d->incomm[i]);
		stc(buf, d->character);
	    }
	    continue;
	}
	if ((d->incomm[i] == 'Y' || d->incomm[i] == 'y') 
		&& ch->pcdata->edit_con == EDIT_CONFIRM_SAVE)
	{
	    char buf[MAX_STRING_LENGTH];
	    int i = 0, j, k = 1;

	    buf[0] = 0;

	    if (!IS_SET(ch->pcdata->edit_flags, EF_NO_OUT)
		&& !IS_SET(ch->pcdata->edit_flags, EF_MENU))
	    {
	    for (j = 0; j < MAX_EDITOR_LINES; j++)
	    {
		if (strlen(ch->pcdata->edit[j]) > 0)
		{
		    if (j > 0)
		    {
			for (i = 0; i < k; i++)
			    strcat(buf, "\n\r");
		    	k = 1;
		    }
		    strcat(buf, ch->pcdata->edit[j]);
		}
		else k += 1;
	    }
	    strcat(buf, "\n\r");

	    if (ch->pcdata->editing != NULL)
	    {
		free_string(*(ch->pcdata->editing));
		*(ch->pcdata->editing) = str_dup(buf);
	    }
	    free_string(ch->pcdata->tempbuf);
	    ch->pcdata->tempbuf = str_dup(buf);
	    ch->pcdata->editing = NULL;
	    }
	    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	    {
		int line;

		for (line = 0; line < MAX_EDITOR_LINES; line++)
		{
		    if (ch->pcdata->menu_flag[line] == MENU_TYPE_END_MARKER)
			break;
		    if (ch->pcdata->menu_flag[line] == MENU_TYPE_DIVIDE)
			continue;
		    if (ch->pcdata->menu_flag[line] == MENU_TYPE_BOOL)
		    {
			if (ch->pcdata->edit[line][40] == '0')
			    (bool *) *(ch->pcdata->menu_variable[line]) = FALSE;
			else
			    (bool *) *(ch->pcdata->menu_variable[line]) = TRUE;
		    }
		    else if (ch->pcdata->menu_flag[line] == MENU_TYPE_FLAG)
		    {
			if (ch->pcdata->edit[line][40] == '0')
			    REMOVE_BIT(*(ch->pcdata->editing_flag), ch->pcdata->menu_ecf[line]);
			else
			    SET_BIT(*(ch->pcdata->editing_flag), ch->pcdata->menu_ecf[line]);
		    }
		    else if (ch->pcdata->menu_flag[line] == MENU_TYPE_INT)
		    {
			if (ch->pcdata->edit[line][40] == 0)
			    (int *) *(ch->pcdata->menu_variable[line]) = 0;
			else
			{
			    int value = atoi(&(ch->pcdata->edit[line][40]));
			    if (value > ch->pcdata->menu_range[line][1]) value = ch->pcdata->menu_range[line][1];
			    if (value < ch->pcdata->menu_range[line][0]) value = ch->pcdata->menu_range[line][0];
			    (int *) *(ch->pcdata->menu_variable[line]) = value;
			}
		    }
		    else if (ch->pcdata->menu_flag[line] == MENU_TYPE_LONG)
		    {
			if (ch->pcdata->edit[line][40] == 0)
			    (long *) *(ch->pcdata->menu_variable[line]) = 0;
			else
			{
			    (long *) *(ch->pcdata->menu_variable[line]) = atoi(&(ch->pcdata->edit[line][40]));
			}
		    }
		    else if (ch->pcdata->menu_flag[line] == MENU_TYPE_SH_INT)
		    {
			    int value = atoi(&(ch->pcdata->edit[line][40]));
			    if (value > ch->pcdata->menu_range[line][1]) value = ch->pcdata->menu_range[line][1];
			    if (value < ch->pcdata->menu_range[line][0]) value = ch->pcdata->menu_range[line][0];
			    (sh_int *) *(ch->pcdata->menu_variable[line]) = value;
		    }
		    else if (ch->pcdata->menu_flag[line] == MENU_TYPE_STRING)
		    {
			free_string( *(ch->pcdata->menu_variable[line]) );
			if (!str_cmp(&(ch->pcdata->edit[line][40]), "(null)"))
			    *(ch->pcdata->menu_variable[line]) = NULL;
			else *(ch->pcdata->menu_variable[line]) = str_dup( &(ch->pcdata->edit[line][40]) );
		    }
		    else
		    {
			bug("Unknown Variable type on line %d.",line);
			bug("Type given denoted by integer %d.",ch->pcdata->menu_flag[line]);
		    }
		}
	    }
	    ch->pcdata->edit_flags = 0;
	    norm_mode_to_char( ch );
	    char_to_game( ch );
	    ansi_cls( ch );
	    ansi_cls( ch );
	    ansi_goto( ch, 1, 1 );
	    if (ch->pcdata->edit_excom != NULL)
		interpret(ch, ch->pcdata->edit_excom);
	    free_string(ch->pcdata->edit_excom);
	    ch->pcdata->edit_excom = NULL;
	    return;
	}
	if ((d->incomm[i] == 'Y' || d->incomm[i] == 'y') 
		&& ch->pcdata->edit_con == EDIT_CONFIRM_CANCEL)
	{
	    norm_mode_to_char( ch );
	    char_to_game( ch );
	    ansi_cls( ch );
	    ansi_cls( ch );
	    ansi_goto( ch, 1, 1 );
	    return;
	}
	if (ch->pcdata->edit_con == EDIT_CONFIRM_SAVE
		|| ch->pcdata->edit_con == EDIT_CONFIRM_CANCEL)
	{
	    if (d->incomm[i] == 'N' || d->incomm[i] == 'n'
		    || d->incomm[i] == 3)
	    {
	    	ch->pcdata->edit_con = EDIT_EDITING;
	    	redraw_editor( d );
	    	continue;
	    }
	    continue;
	}
	if (d->incomm[i] == 24)
	{
	    editor_mesg("Exit and save? (y/n)", ch);
	    ch->pcdata->edit_con = EDIT_CONFIRM_SAVE;
	    continue;
	}
	if (d->incomm[i] == 3)
	{
	    editor_mesg("Exit without saving? (y/n)", ch);
	    ch->pcdata->edit_con = EDIT_CONFIRM_CANCEL;
	    continue;
	}
	if (d->incomm[i] == 20)
	{
	    editor_mesg("Key Test Mode - CTRL-C to return to editor", ch);
	    ch->pcdata->edit_con = EDIT_TEST;
	    continue;
	}
	if (d->incomm[i] == 1)
	{
	    ch->pcdata->csrx = 0;
	    ch->pcdata->csrrx = 0;
	    continue;
	}
	if (d->incomm[i] == 5)
	{
	    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	    {
	    	ch->pcdata->csrx = strlen(&(ch->pcdata->edit[y_pos(ch)][40]));
	    	ch->pcdata->csrrx = strlen(&(ch->pcdata->edit[y_pos(ch)][40]));
	    }
	    else
	    {
	    	ch->pcdata->csrx = strlen(ch->pcdata->edit[y_pos(ch)]);
	    	ch->pcdata->csrrx = strlen(ch->pcdata->edit[y_pos(ch)]);
	    }
	    continue;
	}
	if (d->incomm[i] == 12)
	{
	    redraw_editor( d );
	    continue;
	}
	if (d->incomm[i] == 11)
	{
	    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	    {
		int clx;
		for (clx = 40; clx < 80; clx++)
		    ch->pcdata->edit[y_pos(ch)][clx] = 0;
		if (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_BOOL
		    || ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_INT
		    || ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_SH_INT
		    || ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_LONG
		    || (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_FLAG && ch->pcdata->menu_esg[y_pos(ch)] == 0))
		    ch->pcdata->edit[y_pos(ch)][40] = '0';
		redraw_line( ch, y_pos(ch), cy_pos(ch) );
		continue;
	    }
	    delete_line(ch, y_pos(ch));
	    continue;
	}
	if (d->incomm[i] == 13 || d->incomm[i] == 10)
	{
	    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	    {
		int nextline;
		bool redraw = FALSE;

		ch->pcdata->csrx = 0;
		ch->pcdata->csrrx = 0;
		if (y_pos(ch) >= MAX_EDITOR_LINES - 1
		 || ch->pcdata->menu_flag[y_pos(ch) + 1] == MENU_TYPE_END_MARKER)
		{
		    stc("\7", ch);
		    continue;
	    	}
		nextline = y_pos(ch) + 1;
		while (nextline < MAX_EDITOR_LINES && ch->pcdata->menu_flag[nextline] == MENU_TYPE_DIVIDE)
		    nextline++;

		if (ch->pcdata->menu_flag[nextline] == MENU_TYPE_DIVIDE)
		{
		    stc("\7", ch);
		    continue;
	    	}
		
		ch->pcdata->csry = nextline;
		while (ch->pcdata->csry > 20)
		{
		    ch->pcdata->csry -= 20;
		    ch->pcdata->scry += 20;
		    redraw = TRUE;
		}

		if (redraw)
		    redraw_editor( d );

		continue;
	    }
	    insert_nl(ch, y_pos(ch), cx_pos(ch));
	    if (y_pos(ch) >= MAX_EDITOR_LINES - 1)
	    {
		stc("\7", ch);
		continue;
	    }
	    ch->pcdata->csry++;
	    ch->pcdata->csrrx = 0;
	    ch->pcdata->csrx = 0;
	    if (ch->pcdata->csry > 19 && y_pos(ch) < MAX_EDITOR_LINES)
	    {
	    	ch->pcdata->csry = 10;
	    	ch->pcdata->scry += 10;
		redraw_editor( d );
	    }
	    else if (ch->pcdata->csry > 19) stc("\7", ch);
	    continue;
	}
	if (d->incomm[i] == 27)
	{
	    if (d->incomm[i + 1] == 91)
	    {
		i += 2;
		if (d->incomm[i] == 65) /* Up */
		{
		    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
		    {
			int nextline;

			ch->pcdata->csrx = 0;
			ch->pcdata->csrrx = 0;
			if (y_pos(ch) < 1)
			{
			    stc("\7", ch);
			    continue;
		    	}
			nextline = y_pos(ch) - 1;
			while (nextline > 0 && ch->pcdata->menu_flag[nextline] == MENU_TYPE_DIVIDE)
			    nextline--;

			if (ch->pcdata->menu_flag[nextline] == MENU_TYPE_DIVIDE)
			{
			    stc("\7", ch);
			    continue;
		    	}
		
			ch->pcdata->csry = nextline - ch->pcdata->scry;
		    }
		    else ch->pcdata->csry--;
		}
		else if (d->incomm[i] == 66) /* Down */
		{
		    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
		    {
			int nextline;

			ch->pcdata->csrx = 0;
			ch->pcdata->csrrx = 0;
			if (y_pos(ch) >= MAX_EDITOR_LINES -1
			 || ch->pcdata->menu_flag[y_pos(ch) + 1] == MENU_TYPE_END_MARKER)
			{
			    stc("\7", ch);
			    continue;
		    	}
			nextline = y_pos(ch) + 1;
			while (nextline > 0 && ch->pcdata->menu_flag[nextline] == MENU_TYPE_DIVIDE)
			    nextline++;

			if (ch->pcdata->menu_flag[nextline] == MENU_TYPE_END_MARKER)
			{
			    stc("\7", ch);
			    continue;
		    	}
			

			if (ch->pcdata->menu_flag[nextline] == MENU_TYPE_DIVIDE)
			{
			    stc("\7", ch);
			    continue;
		    	}
		
			ch->pcdata->csry = nextline - ch->pcdata->scry;
		    }
		    else
		    	ch->pcdata->csry++;
		}
		else if (d->incomm[i] == 67) /* Right */
		{
			if (IS_SET(ch->pcdata->edit_flags, EF_MENU)
			    && (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_FLAG
			    || ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_FLAG))
			{
			    stc("\7", ch);
			    continue;
			}
			if (ch->pcdata->edit[y_pos(ch)][ch->pcdata->csrx+1] != 0)
			{
			    ch->pcdata->csrx = ch->pcdata->csrrx;
			    ch->pcdata->csrx++;
			}
		}
		else if (d->incomm[i] == 68) /* Left */
		{
			ch->pcdata->csrx = ch->pcdata->csrrx;
			ch->pcdata->csrx--;
		}
		else if (d->incomm[i] < 32)
		    SET_BIT(ch->pcdata->edit_flags, EF_FIX);

		if (ch->pcdata->csrx < 0)
		{
		    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
		    {
			ch->pcdata->csrx = 0;
			stc("\7", ch);
			continue;
		    }
		    if (y_pos(ch) > 0)
		    {
		    	ch->pcdata->csrx = 78;
		    	ch->pcdata->csry--;
		    }
		    else stc("\7",ch);
		}
		if (ch->pcdata->csrx > 78)
		{
		    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
		    {
			ch->pcdata->csrx = 0;
			stc("\7", ch);
			continue;
		    }
		    if (y_pos(ch) < MAX_EDITOR_LINES)
		    {
		    	ch->pcdata->csrx = 0;
		    	ch->pcdata->csry++;
		    }
		    else stc("\7",ch);
		}
		if (ch->pcdata->csry < 0)
		{
		    if (y_pos(ch) > 0)
		    {
		        ch->pcdata->csry = 10;
			ch->pcdata->scry -= 10;
			if (ch->pcdata->scry < 0)
			{
			    ch->pcdata->csry = 0;
			    ch->pcdata->scry = 0;
			}
			redraw_editor( d );
		    }
		    else
		    {
			stc("\7", ch);
			ch->pcdata->csry = 0;
		    }
		}
		if (ch->pcdata->csry > 19)
		{
		    if (y_pos(ch) < MAX_EDITOR_LINES)
		    {
		    	ch->pcdata->csry = 10;
		    	ch->pcdata->scry += 10;
			redraw_editor( d );
		    }
		    else stc("\7", ch);
		}
		continue;
	    }
	    else
	    {
		SET_BIT(ch->pcdata->edit_flags, EF_FIX);
		continue;
	    }
	}

	if (d->incomm[i] == 127)
	{
	    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	    {
		if (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_BOOL
		    || (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_FLAG && ch->pcdata->menu_esg[y_pos(ch)] == 0))
		{
		    ch->pcdata->menu_flag[y_pos(ch)] = '0';
		    redraw_line(ch, y_pos(ch), cy_pos(ch));
		}
	    }
	    if (ch->pcdata->csrx == 0)
	    {
		stc("\7", ch);
		continue;
	    }
	    ch->pcdata->csrx -= 1;
	    ch->pcdata->csrrx -= 1;
	    delete_text(ch, y_pos(ch), ch->pcdata->csrrx);
	    continue;
	}

	if (d->incomm[i] < 32 || d->incomm[i] > 126)
	{
	    stc("\7",ch);
	    continue;
	}
	if (IS_SET(ch->pcdata->edit_flags, EF_MENU) && (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_BOOL || ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_FLAG))
	{
	    int ln, esg;

	    if (d->incomm[i] != 'x'
	        && d->incomm[i] != 'X'
	        && d->incomm[i] != ' ')
	    {
		stc("\7", ch);
		return;
	    }

	    if (ch->pcdata->edit[y_pos(ch)][40] == '0')
	    {
	    	esg = ch->pcdata->menu_esg[y_pos(ch)];
		if (esg == 0) esg = -1;
		for (ln = 0; ln < MAX_EDITOR_LINES; ln++)
		{
		    if (ch->pcdata->menu_flag[ln] == MENU_TYPE_FLAG && ch->pcdata->menu_esg[ln] == esg 
			&& ch->pcdata->edit[ln][40] != '0')
		    {
			ch->pcdata->edit[ln][40] = '0';
		        redraw_line(ch, ln, ch->pcdata->scry + ln );
		    }
		}
	    	ch->pcdata->edit[y_pos(ch)][40] = '1';
	    }
	    else
	    {
		if (ch->pcdata->menu_esg[y_pos(ch)] == 0)
	     	    ch->pcdata->edit[y_pos(ch)][40] = '0';
	    }
	    redraw_line(ch, y_pos(ch), cy_pos(ch));
	    continue;
	}
	if (IS_SET(ch->pcdata->edit_flags, EF_MENU) 
		&& (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_SH_INT
		|| ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_INT
		|| ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_LONG))
	{
	    if (!isdigit(d->incomm[i]))
	    {
		stc("\7", ch);
		continue;
	    }
	}
	insert_text(ch, y_pos(ch), cx_pos(ch), d->incomm[i]);
	redraw_character( ch );
	ch->pcdata->csrx = ch->pcdata->csrrx + 1;

	// check cursor updates ok
		if (ch->pcdata->csrx < 0)
		{
		    if (y_pos(ch) > 0)
		    {
		    	ch->pcdata->csrx = 78;
		    	ch->pcdata->csry--;
		    }
		    else stc("\7",ch);
		}
		if (ch->pcdata->csrx > 78)
		{
		    if (y_pos(ch) < MAX_EDITOR_LINES)
		    {
		    	ch->pcdata->csrx = 0;
		    	ch->pcdata->csry++;
		    }
		    else stc("\7",ch);
		}
		if (ch->pcdata->csry < 0)
		{
		    if (y_pos(ch) > 0)
		    {
		        ch->pcdata->csry = 10;
			ch->pcdata->scry -= 10;
			if (ch->pcdata->scry < 0)
			{
			    ch->pcdata->csry -= ch->pcdata->scry;
			    ch->pcdata->scry = 0;
			}
			redraw_editor( d );
		    }
		    else stc("\7", ch);
		}
		if (cy_pos(ch) > 19)
		{
		    if (y_pos(ch) < MAX_EDITOR_LINES)
		    {
		    	ch->pcdata->csry = 10;
		    	ch->pcdata->scry += 10;
			redraw_editor( d );
		    }
		    else stc("\7", ch);
		}
    }
    d->incomm[0] = '\0';

    redraw_cursor( ch );
}

void delete_text(CHAR_DATA *ch, int y, int x)
{
    int i;

    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	x += 40;

    if (x <= -1)
    {
	int len, j, k;
	bool delline = FALSE;

	stc("\7", ch);
	if (y <= 0)
	{
	    stc("\7", ch);
	    return;
	}

	i = strlen(ch->pcdata->edit[y-1]) - 2;
	len = 80 - i;
	if (len > strlen(ch->pcdata->edit[y]))
	{
	    len = strlen(ch->pcdata->edit[y]);
	    delline = TRUE;
	}
	ch->pcdata->csry = y - 1;
	if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	{
	    ch->pcdata->csrrx = i - 40;
	    ch->pcdata->csrx  = i - 40;
	}
	{
	    ch->pcdata->csrrx = i;
	    ch->pcdata->csrx = i;
	}
	for (j = 0; j < len; j++)
	{
	    ch->pcdata->edit[y-1][i + j] = ch->pcdata->edit[y][0];
	    for (k = 0; ch->pcdata->edit[y][k+1] != 0; k++)
		ch->pcdata->edit[y][k] = ch->pcdata->edit[y][k+1];
	    ch->pcdata->edit[y][k] = 0;
	}
	if (delline) delete_line(ch, y+1);
	redraw_editor(ch->desc);
	return;
    }

    for (i = x; ch->pcdata->edit[y][i+1] != 0; i++)
	ch->pcdata->edit[y][i] = ch->pcdata->edit[y][i+1];
    ch->pcdata->edit[y][i] = 32;
    ch->pcdata->edit[y][i+1] = 0;
    redraw_line(ch, y, y - ch->pcdata->scry);    
    ch->pcdata->edit[y][i] = 0;
}

void delete_line(CHAR_DATA *ch, int y)
{
    int j;

    for (j = y; j < MAX_EDITOR_LINES; j++)
	strcpy(ch->pcdata->edit[j], ch->pcdata->edit[j+1]);
    ch->pcdata->edit[MAX_EDITOR_LINES - 1][0] = 0;
    redraw_editor(ch->desc);
}


void insert_nl(CHAR_DATA *ch, int y, int x)
{
    int i, j;

    for (j = MAX_EDITOR_LINES - 1; j > y+1; j--)
	strcpy(ch->pcdata->edit[j], ch->pcdata->edit[j-1]);
    j = 0;
    for (i = x; ch->pcdata->edit[y][i] != 0; i++)
    {
	ch->pcdata->edit[y+1][j++] = ch->pcdata->edit[y][i];
	ch->pcdata->edit[y][i] = 0;
    }
    ch->pcdata->edit[y+1][j] = 0;
    redraw_editor( ch->desc );
}

void insert_text(CHAR_DATA *ch, int y, int x, char c)
{
    int i;

    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	x += 40;

    if (c == '~')
    {
	insert_text(ch, y, x, '#');
	insert_text(ch, y, x+1, '-');
	return;
    }

    if (ch->pcdata->edit[y][x] == 0)
    {
	if (IS_SET(ch->pcdata->edit_flags, EF_MENU) && x > 75)
	{
	    stc("\7", ch);
	    return;
	}
	ch->pcdata->edit[y][x] = c;
	ch->pcdata->edit[y][x+1] = 0;
	redraw_charac( ch, x, y, x, y - ch->pcdata->scry );
	redraw_cursor( ch );
    }
    else
    {
    //shift text along one char

    for (i = x; ch->pcdata->edit[y][i] != 0; i++);
    for ( ; i > x; i--)
    	ch->pcdata->edit[y][i] = ch->pcdata->edit[y][i-1];
    ch->pcdata->edit[y][x] = c;
    if (strlen(ch->pcdata->edit[y]) > (IS_SET(ch->pcdata->edit_flags, EF_MENU) ? 38 : 75))
    {
	if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
	{
	    ch->pcdata->edit[y][75] = 0;
	}
	else
	{
	    insert_text(ch, y+1, 0, ch->pcdata->edit[y][75]);
	    ch->pcdata->edit[y][75] = 0;
	}
    }
    }
    redraw_line(ch, y, y - ch->pcdata->scry);
}

void do_edit(CHAR_DATA *ch, char **editing, char *excom, char *title)
{
    char * start;
    int i,j,k;

    start = *editing;

    if (IS_NPC(ch) || ch->desc == NULL || ch->pcdata == NULL) return;

    char_from_game( ch, CON_EDITOR );
    edit_mode_to_char( ch );

    for (i = 0; i < 79; i++)
	for (j = 0; j < 21; j++)
	   ch->pcdata->edit[j][i] = 0;

    i = 0; j = 0;

    if (start != NULL)
    {
        for (k = 0; start[k] != '\0'; k++)
        {
	    if (start[k] == 9)
	    {
		ch->pcdata->edit[j][i++] = ' ';
		ch->pcdata->edit[j][i++] = ' ';
		ch->pcdata->edit[j][i++] = ' ';
		ch->pcdata->edit[j][i++] = ' ';
		continue;
	    }
	    if (start[k] == '\n' || start[k] == '\r')
 	    {
	        k++;
	        i = 0; j++;
	        continue;
	    }
	    ch->pcdata->edit[j][i++] = start[k];
	    if (i > 78) { i = 0; j++; } 
	    if (j > MAX_EDITOR_LINES) break;
        }
    }

    ch->pcdata->csrx = 0;
    ch->pcdata->csrrx = 0;
    ch->pcdata->csry = 0;
    ch->pcdata->scry = 0;
    ch->pcdata->editing = editing;
    ch->pcdata->editing_flag = NULL;
    ch->pcdata->edit_con = EDIT_INITIALISING;
    free_string(ch->pcdata->edit_excom);
    if (excom != NULL)
	ch->pcdata->edit_excom = str_dup(excom);
    else
	ch->pcdata->edit_excom = NULL;
    free_string(ch->pcdata->editor_title);
    if (title == NULL)
	ch->pcdata->editor_title = str_dup("--==> On-Line Editor - (C) 1997 Chris Mellish (Myurr) <==--");
    else
	ch->pcdata->editor_title = str_dup(title);
    ch->pcdata->edit_flags = 0;
    redraw_editor( ch->desc );
}

void manual_edit(CHAR_DATA *ch, char *excom, char *title)
{
    if (IS_NPC(ch) || ch->desc == NULL || ch->pcdata == NULL) return;

    char_from_game( ch, CON_EDITOR );
    edit_mode_to_char( ch );

    ch->pcdata->csrx = 0;
    ch->pcdata->csrrx = 0;
    ch->pcdata->csry = 0;
    ch->pcdata->scry = 0;
    ch->pcdata->editing = NULL;
    ch->pcdata->edit_con = EDIT_INITIALISING;
    free_string(ch->pcdata->edit_excom);
    if (excom != NULL)
	ch->pcdata->edit_excom = str_dup(excom);
    else
	ch->pcdata->edit_excom = NULL;
    free_string(ch->pcdata->editor_title);
    if (title == NULL)
	ch->pcdata->editor_title = str_dup("--==> On-Line Editor - (C) 1997 Chris Mellish (Myurr) <==--");
    else
	ch->pcdata->editor_title = str_dup(title);
    REMOVE_BIT(ch->pcdata->edit_flags, EF_FIX);
    SET_BIT(ch->pcdata->edit_flags, EF_NO_OUT);
    redraw_editor( ch->desc );
}

void redraw_editor( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->character;
    int i;

    if (ch == NULL) return;
    ansi_cls( ch );
    ansi_goto( ch, 1, 1);
    ansi_cls( ch ); 
    editor_title(ch->pcdata->editor_title,ch);
    editor_mesg("CTRL-X to save and exit     CTRL-C to exit    CTRL-L to redraw",ch);
    ansi_goto( ch, 1, 3);
    for (i = 0; i < 20; i++)
	redraw_line( ch, i + ch->pcdata->scry, i );
    redraw_cursor( ch );
}

void redraw_line( CHAR_DATA *ch, int y, int cy )
{
    int cx;

    if (y >= MAX_EDITOR_LINES || cy < 0 || cy > 19)
	return;

    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
    {
	ansi_goto(ch, 1, cy + 3);
	stc( ch->pcdata->edit[y], ch);
	ansi_goto(ch, 40, cy + 3);
	for (cx = 0; cx < 39; cx++)
	    stc(" ", ch);
	ansi_goto(ch, 40, cy + 3);
	if (ch->pcdata->menu_flag[y] == MENU_TYPE_BOOL || ch->pcdata->menu_flag[y] == MENU_TYPE_FLAG)
	{
	    if (ch->pcdata->edit[y][40] == '0')
		stc("[ ]", ch);
	    else stc("[X]", ch);
	}
	else stc( &(ch->pcdata->edit[y][40]), ch);
    }
    else
    {
	ansi_goto(ch, 1, cy + 3);
	stc( ch->pcdata->edit[y], ch);
    }

    redraw_cursor( ch );
}

void redraw_cursor( CHAR_DATA *ch )
{
    if (IS_SET(ch->pcdata->edit_flags, EF_MENU))
    {
	if (ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_BOOL || ch->pcdata->menu_flag[y_pos(ch)] == MENU_TYPE_FLAG)	
	    ansi_goto(ch, 41, cy_pos(ch) + 3);
	else
	    ansi_goto(ch, cx_pos(ch) + 40, cy_pos(ch) + 3);
    }
    else
	ansi_goto(ch, cx_pos(ch) + 1, cy_pos(ch) + 3);
}

void redraw_character( CHAR_DATA *ch )
{
    char buf[2];

    buf[0] = ch->pcdata->edit[y_pos(ch)][cx_pos(ch) - 1];
    buf[1] = 0;
    ansi_goto(ch, cx_pos(ch), cy_pos(ch) + 3);
    stc(buf, ch);
}

void redraw_charac( CHAR_DATA *ch, int x, int y, int cx, int cy )
{
    char buf[2];

    buf[0] = ch->pcdata->edit[y][x];
    buf[1] = 0;
    ansi_goto(ch, cx, cy + 3);
    stc(buf, ch);
}

void editor_title( char *txt, CHAR_DATA *ch )
{
    char buf[90];
    char buf2[90];
    int i = strlen(txt), j;

    if (ch == NULL || txt == NULL) return;
    if (i > 65) {txt[65] = '\0'; i = 65;}
    buf[0] = '#'; buf[1] = 'i';
    for (j = 0; j < ((79 - i) / 2); j++)
	buf[j] = ' ';
    buf[j] = '\0';
    sprintf(buf2, "#I%s%s%s#N", buf, txt, buf);
    ansi_goto(ch, 1, 1);
    send_to_char(buf2, ch);
}

void editor_mesg( char *txt, CHAR_DATA *ch )
{
    char buf[90];
    char buf2[90];
    int i = strlen(txt), j;

    if (ch == NULL || txt == NULL) return;
    if (i > 65) {txt[65] = '\0'; i = 65;}
    buf[0] = '#'; buf[1] = 'i';
    for (j = 0; j < ((79 - i) / 2); j++)
	buf[j] = ' ';
    buf[j] = '\0';
    sprintf(buf2, "#I%s%s%s#N", buf, txt, buf);
    ansi_goto(ch, 1, 24);
    send_to_char(buf2, ch);
}


void char_from_game( CHAR_DATA *ch, int con )
{
    CHAR_DATA *chl;
    char buf[MAX_INPUT_LENGTH];

    if (ch != NULL && ch->in_room != NULL)
    {
	ch->desc->prevroom = ch->in_room->vnum;
	if (con == CON_EDITOR)
		sprintf(buf, "%s has entered the editor.\n\r",GET_NAME(ch));
	else
		sprintf(buf, "%s has left the game.\n\r",GET_NAME(ch));
    	for (chl = ch->in_room->people; chl != NULL; chl = chl->next_in_room)
	    if (chl != ch) stc(buf, chl);
    }


    char_from_room( ch );
    if (char_list == ch)
	char_list = ch->next;
    else
    {
	CHAR_DATA *wch;
	for (wch = char_list; wch != NULL; wch = wch->next)
	{
	    if (wch->next == ch)
	    {
		wch->next = ch->next;
		ch->next = NULL;
	    }
	}
    }

    ch->desc->connected = con;
}

void char_to_game( CHAR_DATA *ch )
{
    CHAR_DATA *chl;
    char buf[MAX_INPUT_LENGTH];

    ch->next  = char_list;
    char_list = ch;
    char_to_room(ch, get_room_index(char_realm(ch), ch->desc->prevroom), 0);
    if (ch->desc->connected == CON_EDITOR)
	sprintf(buf, "%s has re-entered the game from the editor.\n\r",GET_NAME(ch));
    else
	sprintf(buf, "%s has re-entered the game.\n\r",GET_NAME(ch));
    ch->desc->connected = CON_PLAYING;
    	for (chl = ch->in_room->people; chl != NULL; chl = chl->next_in_room)
	    if (chl != ch) stc(buf, chl);
}

void game_crash( int sig )
{
    FILE *fp;
    char * fault;
    char buf[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *ch;

    fault = "none";

    switch(sig)
    {
	case SIGALRM : fault = "Caught Infinite Looping"; break;
	case SIGHUP  : fault = "SIGHUP"; break;
	case SIGSEGV : fault = "Segmentation Fault"; break;
	case SIGTERM : fault = "Task Killed"; break;
    }

 // rest of the cack

  if (!debug_dont_run)
  {
    debug_dont_run = TRUE;

    if ((fp = fopen("crash.details", "a"))!=NULL)
    {
	int i;

	fprintf(fp, "*** CRASH DETAILS OF LAST CRASH ***\n\n");
	fprintf(fp, "Last command executed was:\n%s\n\nAnd was entered by %s\n\n",
	    debug_last_command,
	    debug_last_character);
	fprintf(fp, "Command was executed in room: %d\n\n",
	    debug_last_room);
	for (i = 0 ; i < 20 ; i++)
	  fprintf(fp, "Procedure %d : %s\n", i, debug_procedure[i]);
	fprintf(fp, "Last proc written to: %d\n\n", debug_counter);

	fprintf(fp, "Crash type : %s\n\n", fault);
	fprintf(fp, "Function stage: %d\n\n", debug_stage);
	fprintf(fp, "***********************************\n\n");
	fclose( fp );
    }

    for (d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	ch = d->original ? d->original : d->character;
	if (ch != NULL && d->connected == CON_PLAYING)
	{
	    int i;

	    sprintf(buf, "#1--#3--#7>#3 Mud crashed - %s - rebooting a.s.a.p. #7<#3--#1--\n\r", fault);
	    stc(buf, ch);
	    for (i = 0 ; i < 20 ; i++)
		stcf(ch, "Procedure number %d was %s.\n\r",
			i, debug_procedure[i]);
	    stcf(ch, "Last proc written to : %d.\n\r", debug_counter);
	    sprintf(buf,"\n\nCommand: %s\n\rPlayer: %s\n\rRoom: %d\n\rBug logged.  Do not repeat.\n\r",
		debug_last_command,
		debug_last_character,
		debug_last_room);
	    stc(buf, ch);
	    close_socket( d );
	}
    }

  }
    exit( 1 );
}

void init_signals( void )
{
#ifndef USE_CORE_FILE
    struct sigaction act;

    act.sa_mask  = (SIGHUP|SIGSEGV|SIGTERM|SIGALRM);
    act.sa_flags = SA_RESTART;

    act.sa_handler = (*game_crash);

    sigaction(SIGHUP,	&act, (struct sigaction * ) 0);
    sigaction(SIGSEGV,	&act, (struct sigaction * ) 0);
    sigaction(SIGTERM,	&act, (struct sigaction * ) 0);
    sigaction(SIGALRM,	&act, (struct sigaction * ) 0);

    signal(SIGCHLD, SIG_DFL);
#endif   
}

void do_email_list( CHAR_DATA *ch, char *argument )
{
    emailing_to = email_list;
}

void email_next()
{
    char buf[MAX_STRING_LENGTH];

    if (emailing_to == NULL || dice(1, 10) != 4) return;

    sprintf(buf, "AutoEmailing %s", emailing_to->address);
    informational(buf, -1, -1);
    sprintf(buf, "mail -s 'Dark Heart II - Automailer' %s < email.mesg&", emailing_to->address);
    system(buf);
    emailing_to = emailing_to->next;
}

void email_character( CHAR_DATA *ch )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];

    if (ch == NULL || ch->email_set != 1 || IS_NPC(ch)) return;

    if ((fp = fopen("email.tmp","w"))==NULL)
	return;

    fprintf(fp, "This is an automatic posting from the BTL mud Dark Heart II\n\n");
    fprintf(fp, "User registration information:\n\n");
    fprintf(fp, "Character Name:  %s\n\n", GET_PROPER_NAME(ch));
    fprintf(fp, "To register your character, log on in the normal way and type:\n\n");
    fprintf(fp, "    register key %d\n\n", ch->registration_key);
    fprintf(fp, "Until this is done, you will not be allowed to save your character.\n");

    fclose( fp );

    sprintf(buf, "mail -s 'Dark Heart II - Automailer' %s < email.tmp&",ch->pcdata->email);
    system(buf);
}

void do_register(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    if (ch->registerred == 1 && !IS_COOL(ch)) 
    {
	stc("You have already registerred.\n\r",ch);
	return;
    }


    if (ch->registration_key < 100000 && !IS_COOL(ch)) ch->registerred = 0;

    if (ch->pcdata->email == NULL)
    {
	stc("You must use the email command to enter your email address first.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	stc("Syntax: register email <address>\n\r",ch);
	stc("        register key <key code>\n\r",ch);
	if (IS_COOL(ch))
	    stc("        register player <name>\n\r",ch);
	return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' ||
	(str_prefix(arg, "email") && str_prefix(arg, "key")
	&& (!IS_COOL(ch) || str_prefix(arg, "player"))))
    {
	stc("Syntax: register email <address>\n\r",ch);
	stc("        register key <key code>\n\r",ch);
	if (IS_COOL(ch))
	    stc("        register player <name>\n\r",ch);
	return;
    }

    if (!str_prefix(arg, "player"))
    {
	CHAR_DATA *wch;

	if (argument[0] == '\0')
	{
	    stc("Overide Registration on which character?\n\r",ch);
	    return;
	}

	if ((wch = get_char_world(ch, argument)) != NULL)
	{
	    wch->registerred = 1;
	    wch->email_set = 2;
	    save_char_obj( wch );
	    divide6_to_char( wch );
	    banner2_to_char("Character Registerred",wch);
	    banner2_to_char("You can  now save",wch);
	    divide6_to_char( wch );
	    stc("They have now been registerred.\n\r",ch);
	    return;
	}
	else
	{
	    stc("They are not playing at the moment.\n\r",ch);
	    return;
	}
    }

    if (!str_prefix(arg, "email"))
    {
	if (ch->email_set != 0)
	{
	    stc("You have already registerred your email address.\n\r",ch);
	    return;
	}

	if (!str_cmp(argument, ch->pcdata->email))
	{
	    if (check_email(argument))
	    {
		stc("That email address has already been taken.\n\r",ch);
		return;
	    }
	    while (ch->registration_key < 100000)
	    {
	   	ch->registration_key = dice(1,899999) + 100000;
	    }
	    ch->email_set = 1;
	    email_character( ch );
	    ch->registerred = 3;
	    save_char_obj( ch );
	    stc("Email address registerred.  Registration Email sent.\n\r",ch);
	    stc("Your character has been saved.\n\r",ch);
	    return;
	}
	else
	{
	    stc("Your two entered email addresses differ.\n\r",ch);
	    stc("The one entered using the email command is:\n      ",ch);
	    stc(ch->pcdata->email,ch);
	    stc("\n\rEither correct using the email command, or enter this address correctly\n\rwith the register command.\n\r",ch);
	    return;
	}
    }

    if (!str_prefix(arg, "key"))
    {
	char buf[MAX_INPUT_LENGTH];

	if (ch->registration_key < 100000)
	{
	    stc("Invalid Registration Key.\n\r",ch);
	    stc("Please re-register.\n\r",ch);
	    ch->email_set = 0;
	    return;
	}

	sprintf(buf, "%d", ch->registration_key);

	if (!str_cmp(argument, buf))
	{
	    ch->registerred = 1;
	    ch->email_set = 2;
	    save_char_obj( ch );
	    add_email_to_list( ch->pcdata->email );
	    divide6_to_char( ch );
	    banner2_to_char("Character Registerred",ch);
	    banner2_to_char("You can  now save",ch);
	    divide6_to_char( ch );
	}
	else
	{
	    stc("The registration key you have enterred is not correct.\n\r",ch);
	}
    }
}

void do_reregister(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;

    if (argument[0] == '\0')
    {
	stc("Syntax:  reregister <character>\n\rUsed if they used an invalid email account for registerring.\n\r", ch);
	return;
    }

    if ((vch = get_char_world(ch, argument)) == NULL)
    {
	stc("They are not logged on.\n\r",ch);
	return;
    }

    vch->registerred = 0;
    vch->email_set = 0;
    stc("They can now register.\n\r",ch);
    stc("You can now attempt to register again.\n\r",vch);
}

void menu_option( CHAR_DATA *ch, int line, char *label, void **var, const char *type, ...)
{
    va_list ap;
    int c;

    va_start(ap, type);

    if (!IS_NPC(ch) && line > -1 && line < MAX_EDITOR_LINES && label != NULL)
    {
	for (c = 0; label[c] != 0; c++)
	{
	    ch->pcdata->edit[line][c] = label[c];
	    if (c > 36) break;
	}
	for ( ; c < 39; c++)
	    ch->pcdata->edit[line][c] = ' ';
	ch->pcdata->edit[line][37] = ':';
	ch->pcdata->edit[line][39] = 0;
 	ch->pcdata->menu_variable[line] = var;
	switch ( type[0] )
	{
	    case 'B' : ch->pcdata->menu_flag[line] = MENU_TYPE_BOOL;   sprintf(&(ch->pcdata->edit[line][40]), "%d", va_arg(ap, bool)); break;
	    case 'C' : ch->pcdata->menu_flag[line] = MENU_TYPE_LETTER; sprintf(&(ch->pcdata->edit[line][40]), "%c", va_arg(ap, char)); break;
	    case 'S' : ch->pcdata->menu_flag[line] = MENU_TYPE_STRING; sprintf(&(ch->pcdata->edit[line][40]), "%s", va_arg(ap, char *)); break;
	    case 'I' : ch->pcdata->menu_flag[line] = MENU_TYPE_INT;    sprintf(&(ch->pcdata->edit[line][40]), "%d", va_arg(ap, int)); 
			ch->pcdata->menu_range[line][0] = va_arg(ap, int); ch->pcdata->menu_range[line][1] = va_arg(ap, int); break;
	    case 'L' : ch->pcdata->menu_flag[line] = MENU_TYPE_LONG;   sprintf(&(ch->pcdata->edit[line][40]), "%ld", va_arg(ap, long)); break;
	    case 'H' : ch->pcdata->menu_flag[line] = MENU_TYPE_SH_INT; sprintf(&(ch->pcdata->edit[line][40]), "%d", va_arg(ap, sh_int));
			ch->pcdata->menu_range[line][0] = va_arg(ap, int); ch->pcdata->menu_range[line][1] = va_arg(ap, int); break;
	    case 'D' : ch->pcdata->menu_flag[line] = MENU_TYPE_DIVIDE; sprintf(ch->pcdata->edit[line], "%s", va_arg(ap, char *)); break;
	    case 'F' : ch->pcdata->menu_flag[line] = MENU_TYPE_FLAG;   ch->pcdata->menu_ecf[line] = va_arg(ap, long); ch->pcdata->menu_esg[line] = va_arg(ap, int); 
		       sprintf(&(ch->pcdata->edit[line][40]), "%ld", IS_SET(*(ch->pcdata->editing_flag), ch->pcdata->menu_ecf[line])); break;
	}
    }

    va_end(ap);
    return;
}

void menu_divider( CHAR_DATA *ch, int line )
{
    sprintf(ch->pcdata->edit[line], "%20s- -- --- -----=====>><=====----- --- --- -", " ");
    ch->pcdata->menu_variable[line] = NULL;
    ch->pcdata->menu_flag[line] = MENU_TYPE_DIVIDE;
}

void menu_init( CHAR_DATA *ch )
{
    int line;

    if (IS_NPC(ch)) return;

    for (line = 0; line < MAX_EDITOR_LINES; line++)
    {
	ch->pcdata->menu_variable[line] = NULL;
	ch->pcdata->menu_flag[line] = MENU_TYPE_END_MARKER;
	ch->pcdata->edit[line][0] = 0;
	ch->pcdata->edit[line][40] = 0;
    }

    ch->pcdata->editing = NULL;
}

void menu_init_flags( CHAR_DATA *ch, long *flg )
{
    int line;

    if (IS_NPC(ch)) return;

    for (line = 0; line < MAX_EDITOR_LINES; line++)
    {
	ch->pcdata->menu_variable[line] = NULL;
	ch->pcdata->menu_flag[line] = MENU_TYPE_END_MARKER;
	ch->pcdata->edit[line][0] = 0;
	ch->pcdata->edit[line][40] = 0;
    }

    ch->pcdata->editing = NULL;
    ch->pcdata->editing_flag = flg;
}


void do_setup( CHAR_DATA *ch, char *argument )
{
    NO_NPC(ch);
    if (!IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->level < 112)
    { 
        stc("You can only change your setup in a safe room.\n\r", ch);
        return;
    }


    menu_init( ch );
    menu_option( ch, 0, "Title", (void **) &ch->pcdata->title, MENU_STRING, ch->pcdata->title);
    menu_option( ch, 1, "Email Address", (void **) &ch->pcdata->email, MENU_STRING, ch->pcdata->email);
    menu_option( ch, 2, "Homepage", (void **) &ch->pcdata->homepage, MENU_STRING, ch->pcdata->homepage);
    menu_option( ch, 3, "Real Name", (void **) &ch->pcdata->real_name, MENU_STRING, ch->pcdata->real_name);
    menu_option( ch, 4, "Plan", (void **) &ch->pcdata->plan, MENU_STRING, ch->pcdata->plan);
    menu_divider( ch, 5 );
    menu_option( ch, 6, "Wimpy", (void **) &ch->wimpy, MENU_SH_INT, ch->wimpy);

    do_menu( ch, "save", "Character Setup" );
}

void do_automenu( CHAR_DATA *ch, char *argument )
{
    NO_NPC(ch);
    if (!IS_SET(ch->in_room->room_flags, ROOM_SAFE) && ch->level < 112)
    { 
        stc("You can only change your setup in a safe room.\n\r", ch);
        return;
    }

    menu_init_flags( ch, &ch->act );
    menu_option( ch, 0, "Autoassist", 	NULL, MENU_FLAG, PLR_AUTOASSIST, 0);
    menu_option( ch, 1, "Autoexit", 	NULL, MENU_FLAG, PLR_AUTOEXIT, 0);
    menu_option( ch, 2, "Autogold", 	NULL, MENU_FLAG, PLR_AUTOGOLD, 0);
    menu_option( ch, 3, "Autoloot",	NULL, MENU_FLAG, PLR_AUTOLOOT, 0);
    menu_option( ch, 4, "Autosac",	NULL, MENU_FLAG, PLR_AUTOSAC, 0);
    menu_option( ch, 5, "Autosplit",	NULL, MENU_FLAG, PLR_AUTOSPLIT, 0);
    menu_option( ch, 6, "Autoscan",	NULL, MENU_FLAG, PLR_AUTOSCAN, 0);
    menu_divider( ch, 7 );
    menu_option( ch, 8, "Refuse Followers", NULL, MENU_FLAG, PLR_NOFOLLOW, 0);

    do_menu( ch, "save", "Auto Functions Setup" );
}

void do_menu(CHAR_DATA *ch, char *excom, char *title)
{
    if (IS_NPC(ch) || ch->desc == NULL || ch->pcdata == NULL) return;

    char_from_game( ch, CON_EDITOR );
    edit_mode_to_char( ch );

    ch->pcdata->csrx = 0;
    ch->pcdata->csrrx = 0;
    ch->pcdata->csry = 0;
    ch->pcdata->scry = 0;
    ch->pcdata->editing = NULL;
    ch->pcdata->edit_con = EDIT_INITIALISING;
    free_string(ch->pcdata->edit_excom);
    if (excom != NULL)
	ch->pcdata->edit_excom = str_dup(excom);
    else
	ch->pcdata->edit_excom = NULL;
    free_string(ch->pcdata->editor_title);
    if (title == NULL)
	ch->pcdata->editor_title = str_dup("--==> On-Line Editor - (C) 1997 Chris Mellish (Myurr) <==--");
    else
	ch->pcdata->editor_title = str_dup(title);
    ch->pcdata->edit_flags = 0;
    SET_BIT(ch->pcdata->edit_flags, EF_MENU);
    redraw_editor( ch->desc );
}

void stcf( CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    if (ch == NULL)
	return;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    write_to_buffer( ch->desc, buf, strlen(buf) );
    return;
}

void console( const char *fmt, ...)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    va_list ap;
    int len;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    len = strlen(buf);
    va_end(ap);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if (d->connected == CON_CONSOLE)
	    write_to_buffer( d, buf, len );
    }
    return;
}

void set_alarm( int secs )
{
    alarm( secs );
}

void do_console( CHAR_DATA *ch, char *argument )
{
    char_from_game( ch, CON_CONSOLE );
}

void recover_descriptor(DESCRIPTOR_DATA *dnew, int desc)
{
    int i;

    dnew->descriptor = desc;
    dnew->character = NULL;
    dnew->connected = CON_GET_NAME;
    dnew->showstr_head = str_dup("");
    dnew->showstr_point = 0;
    dnew->outsize = 2000;
    dnew->outbuf = alloc_mem(dnew->outsize);
    dnew->inlasti       = 0;
    dnew->value         = 0;
    dnew->sent_bytes[0] = 0;
    dnew->sent_bytes[1] = 0;
    dnew->sent_bytes[2] = 0;
    dnew->sent_secs[0]  = 0;
    dnew->sent_secs[1]  = 0;
    dnew->sent_pulse    = 0;
      
    for (i = 0; i < 9; i++)  
    {  
        dnew->inlast[i][0] = '\0';
    }   
}

void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    int desc;
    int size;
    int i;
                 
    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {    
        perror( "New_descriptor: accept" );
        return;
    }    
           
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif
 
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
        perror( "New_descriptor: fcntl: FNDELAY" );
        return;
    }

    /*   
     * Cons a new descriptor.
     */
    dnew = new_descriptor();
     
    dnew->descriptor    = desc;
    dnew->connected     = CON_GET_NAME;
    dnew->showstr_head  = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize       = 2000;
    dnew->outbuf        = alloc_mem( dnew->outsize );
    dnew->inlasti       = 0;
    dnew->value         = 0;
    dnew->sent_bytes[0] = 0;
    dnew->sent_bytes[1] = 0;
    dnew->sent_bytes[2] = 0;
    dnew->sent_secs[0]  = 0;
    dnew->sent_secs[1]  = 0;
    dnew->sent_pulse    = 0;
      
    for (i = 0; i < 9; i++)  
    {  
        dnew->inlast[i][0] = '\0';
    }
    
    dnew->next                  = descriptor_list;
    descriptor_list             = dnew;
    
    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
        perror( "New_descriptor: getpeername" );
        dnew->host = str_dup( "(unknown)" );
    }
    else
    {
        /*
         * Would be nice to use inet_ntoa here but it takes a struct arg,
         * which ain't very compatible between gcc and system libraries.
         */
        int addr;
        struct hostent *from;
        
        addr = ntohl( sock.sin_addr.s_addr );
        sprintf( buf, "%d.%d.%d.%d",   
            ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
            ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
            );
        sprintf( log_buf, "Sock.sinaddr:  %s", buf );
        log_string( log_buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );   
    }
   
    dnew->connected = CON_GET_NAME;
    init_signals(); 				
    if ( check_ban(dnew->host,BAN_ALL))
    {
        write_to_descriptor( desc,
            "Your site has been banned from this mud.\n\r", 0 );
        close( desc );
        free_descriptor(dnew);  
        return;
    }
    return;
}

void logf (char * fmt, ...)                                                    
{                                                                             
        char buf [2*MSL];                                                    
        va_list args;                                                        
        va_start (args, fmt);                                                
        vsprintf (buf, fmt, args);                                           
        va_end (args);                                                       
                                                                             
        log_string (buf);                                                     
}                                                                             

void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100];
	
	fp = fopen (COPYOVER_FILE, "w");
	
	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		logf ("Could not write to copyover file: %s", COPYOVER_FILE);
		perror ("do_copyover:fopen");
		return;
	}
	
	/* Consider changing all saved areas here, if you use OLC */
	
	/* do_asave (NULL, ""); - autosave changed areas */
	
	
	sprintf (buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", ch->name);
	
	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->character || d->connected != CON_PLAYING) /* drop those logging on */
		{
			write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few seconds.\n\r", 0);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
			save_char_obj (och);
			write_to_descriptor (d->descriptor, buf, 0);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	/* Close reserve and other always-open files and release other resources */
	
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXEC_FILE, "BTL", buf, "copyover", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);
	
	/* Here you might want to reopen fpReserve */
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100];
	char host[MSL];
	int desc;
	bool fOld;
	
	logf ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		logf ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */		
		if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r",0))
		{
			close (desc); /* nope */
			continue;
		}
		
		d = alloc_perm (sizeof(DESCRIPTOR_DATA));
		recover_descriptor (d,desc); /* set up various stuff */
		
		d->host = str_dup (host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_READ_MOTD;
		
	
		/* Now, find the pfile */
		
		fOld = load_char_obj (d, name);
		
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else /* ok! */
		{
			write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r",0);
	
			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index (prime_realm, ROOM_VNUM_TEMPLE);

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;

			char_to_room (d->character, d->character->in_room, 0);
			do_look (d->character, "");
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;
		}
		
	}
	
	
}
