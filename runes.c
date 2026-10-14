/*****************************************************************************
*  Adam Powell - Rune.C    This should keep the French happy!		     *
*****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "recycle.h"
#include "magic.h"

/* Declare local variables right now I think */

DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_say		);

bool know_rune		args( (CHAR_DATA *ch, sh_int rune) );
int  get_rune_power 	args( (CHAR_DATA *ch) );
int  get_rune_number    args( (char runeletter) );
char * get_rune_name	args( (int rune) );

char * rune_name [10] =
{
    "dyn", "toi", "yma", "melc", "vict",
    "elb", "ger", "bia", "log", "zoe"
};


/* Check if they know a rune at all! */
bool know_rune(CHAR_DATA *ch, sh_int rune)
{
    if (get_rune_power(ch) < rune) return FALSE;
    return TRUE;
}


/* Ok, Warlocks and Eladrin may do it! */
int get_rune_power(CHAR_DATA *ch)
{
    if (ch->level == 120) return 10;
    if (ch->secondaryrace == RACE_WARLOCK) return DISC_WARL_RUNE;
    if (ch->secondaryrace == RACE_ELADRIN) return DISC_ELAD_MELO;
    return -1;
}

char * get_rune_name(int rune)
{
    if (rune == -1) return "WrOnG";
    return rune_name[rune-1];
}

void rune_cast_effect(CHAR_DATA *ch, char *argument, sh_int rune1, sh_int rune2, sh_int rune3)
{
    sh_int temprune;
    char buf[100];

    /* This bit sorts the runes out -
	into rune order, and eliminates repeating runes */
    if (rune1 == rune2) rune2 = -1;
    if (rune2 == rune3) rune3 = -1;
    if (rune1 == rune3) rune3 = -1;

    /* Now sort the runes into order using a tempbuf */
    if (rune1 < rune3)
    {
	temprune = rune3;
	rune3 = rune1;
	rune1 = temprune;
    }

    if (rune1 < rune2)
    {
	temprune = rune2;
	rune2 = rune1;
	rune1 = temprune;
    }

    if (rune2 < rune3)
    {
	temprune = rune3;
	rune3 = rune2;
	rune2 = temprune;
    }

    /* Said three totally wrong runes! */
    if (rune1 == -1)
    {
	WAIT_STATE(ch, 8);
	do_say(ch, "I seem to have forgotten how to use runes!");
	return;
    }

    /* Only one rune cast */
    else if (rune2 == -1)
    {
	WAIT_STATE(ch, 7);
	sprintf(buf, "%s", get_rune_name(rune1) );
	do_say(ch, buf);

	/* One rune effect?   maybe  */
	return;
    }

    /* Two runes cast */
    else if (rune3 == -1)
    {
	WAIT_STATE(ch, 7);
	sprintf(buf, "%s %s", get_rune_name(rune1), get_rune_name(rune2) );
	do_say(ch, buf);

	/* Two rune effect?   why not!  */
	return;
    }

    /* All runes cast */
    else
    {
	WAIT_STATE(ch, 7);
	sprintf(buf, "%s %s %s", get_rune_name(rune1), get_rune_name(rune2),
		get_rune_name(rune3) );
	do_say(ch, buf);

	/* Ok, three runes is the hard bit!  */
	return;
    }

}

int get_rune_number(char runeletter)
{
    runeletter = LOWER(runeletter);

    if (runeletter == 'd') return 1;
    if (runeletter == 't') return 2;
    if (runeletter == 'y') return 3;
    if (runeletter == 'm') return 4;
    if (runeletter == 'v') return 5;
    if (runeletter == 'e') return 6;
    if (runeletter == 'g') return 7;
    if (runeletter == 'b') return 8;
    if (runeletter == 'l') return 9;
    if (runeletter == 'z') return 10;

    return -1;
}


/* The actual interpreter command for runes */

void do_runes(CHAR_DATA *ch, char *argument)
{
    sh_int rune1 = -1, rune2 = -1, rune3 = -1;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
	stc("Syntax : runes [list cast]\n\r", ch);
	return;
    }
 
    if (!str_prefix(arg1, "cast"))
    {
	if (arg2[0] == '\0')
	{
	    stc("What runes do you wish to use?\n\r", ch);
	    return;
	}

	if (strlen(arg2) != 666)
	{
	    rune1 = get_rune_number(arg2[0]);
	    if (arg2[1] != '\0') rune2 = get_rune_number(arg2[1]);
	    if (arg2[2] != '\0') rune3 = get_rune_number(arg2[2]);
	    rune_cast_effect(ch,argument, rune1, rune2, rune3);
	    return;
	}
    }

    if (!str_prefix(arg1, "list"))
    {
	if (get_rune_power(ch) == 0)
	{
	    stc("You do not know any runes.\n\r", ch);
	    return;
 	}

	ctc("Runes Known", ch);
	ctc("---==> <==--=", ch);
	stc("\n\r", ch);
	ctc("Dyn - the personal rune.", ch);
	if (get_rune_power(ch) >=2)
	    ctc("Toi - the foe rune.", ch);
	if (get_rune_power(ch) >=3)
	    ctc("Yma - the electric rune.", ch);
	if (get_rune_power(ch) >=4)
	    ctc("Melc - the luck rune.", ch);
	if (get_rune_power(ch) >=5)
	    ctc("Vict - the healing rune.", ch);
	if (get_rune_power(ch) >=6)
	    ctc("Elb - the cold rune.", ch);
	if (get_rune_power(ch) >=7)
	    ctc("Ger - the heat rune.", ch);
	if (get_rune_power(ch) >=8)
	    ctc("Bia - the negation rune.", ch);
	if (get_rune_power(ch) >=9)
	    ctc("Log - the transference rune.", ch);
	if (get_rune_power(ch) >=10)
	    ctc("Zoe - the death rune.", ch);
	return;
    }

    do_runes(ch, "");
    return;
}
