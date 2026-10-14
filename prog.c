/*************************************************************                
*     Mob Progs - Module Version 2.00 (C) Chris Mellish      *   
* +--------------------------------------------------------+ *
*     Interpreter       - written by Chris Mellish           *
*     Misc Functions    - written by Chris Mellish           *
*                         and his pet marrigold Adam Powell  *
*       Trigger Code    - written by Chris Mellish           *
*             &             and Adam Powell                  *  
*     Default Functions                                      *
*************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hell.h"
#include "prog.h"


void crunch_fn 	args( (PROG_DATA *prog, char *fn, char *args, char *retbuf) );
void retn	args( (char *buf, bool state) );
void clip_quotes args( ( char *buf ) );

/*************************************************************                
* Check Trigger exists					     *
*************************************************************/

void check_trigger_room ( CHAR_DATA *ch, int trigger, char *argument )
{
    CHAR_DATA *vch,*chnext;

    if (ch->in_room == NULL)
	return;

    for (vch = ch->in_room->people; vch != NULL; vch = chnext)
    {
	chnext = vch->next_in_room;
	if (vch->altitude != ch->altitude) continue;
	if (vch != ch)
	    check_trigger(ch, vch, trigger, argument);
    }
}

void check_trigger ( CHAR_DATA *ch, CHAR_DATA *vch, int trigger, char *argument )
{
    if (vch->all_trig_claim
	&& vch->attached_trig[trigger] == NULL
	&& vch->halted_trig[trigger] == NULL)
	return;

    if (vch->halted_trig[trigger] != NULL
	&& (vch->latch_halted[trigger] == FALSE || ch == vch->halted_trig[trigger]->trig))
    {
	resume_prog(vch, ch, argument, trigger, vch->halted_trig[trigger]);
	return;
    }

    if (vch->attached_trig[trigger] != NULL)
    {
	init_prog(vch, ch, argument, vch->attached_trig[trigger], trigger);
	return;
    }

    if (vch->pIndexData == NULL)
	return;    

    if (!ch->run_trig[trigger] && vch->pIndexData->trig[trigger] != NULL)
    {
	init_prog(vch, ch, argument, vch->pIndexData->trig[trigger], trigger);
    }
    /* else run default prog according to mob type etc. */
}

void resume_prog(CHAR_DATA *mob, CHAR_DATA *trg, char *data, int trigger, PROG_DATA *prog)
{
    prog->ch = mob;
    mob->run_trig[trigger] = TRUE;
    prog->trig = trg;
    prog->trigger = trigger;
    prog->pause = 0;
    prog->frozen = 0;
    if (data != NULL)
	strcpy(prog->r[7], data);
    else
	prog->r[7][0] = '\0';
    mob->run_trig[trigger] = TRUE;
    run_prog( prog );
}


void init_prog(CHAR_DATA *mob, CHAR_DATA *trg, char *data, MOB_PROG_DATA *code, int trigger)
{
    PROG_DATA *prog;
    int i;

    prog = new_prog();

    prog->ch = mob;
    mob->run_trig[trigger] = TRUE;
    prog->trig = trg;
    prog->trigger = trigger;
    prog->running = code;
    for (i = 0; i < 8; i++)
	prog->r[i][0] = '\0';
    for (i = 0; i < 20; i++)
	prog->whilestack[i] = -1;
    prog->whilelevel = 0;
    prog->iflevel = 0;
    prog->pause = 0;
    prog->frozen = 0;
    prog->cur_line = 0;
    if (data != NULL)
	strcpy(prog->r[7], data);
    else
	prog->r[7][0] = '\0';
    mob->run_trig[trigger] = TRUE;
    run_prog( prog );
}

PROG_DATA *new_prog()
{
    PROG_DATA *prog;

    prog = alloc_mem( sizeof(*prog) );

    prog->next = prog_list;
    prog_list = prog;
    return prog;
}

void do_validate_prog(CHAR_DATA *ch, char *argument)
{
    MOB_PROG_DATA *prog;

    if (argument[0] == 0)
    {
	stc("Validate which mob prog?\n\r", ch);
	return;
    }

    if ((prog = get_prog_index( argument )) == NULL)
    {
	stc("No such procedure has been defined.\n\r", ch);
	return;
    }

    validate_prog(ch, prog);
}

void validate_prog( CHAR_DATA *ch, MOB_PROG_DATA *prog )
{
    int line;
    int chr, count, myerc = 0;
    bool in_quote = FALSE;

    prog->valid = TRUE;

    /* Scan lines */

    stcf(ch, "Compiling program: %s\n\r", prog->proc_name);

    for (line = 0; line < MAX_MOB_PROG_LINES; line++)
    {
	if (prog->line[line] == NULL)
	    break;

/******** Count ( )'s, check "'s match, and validate substitutions ********/
	count = 0;
	in_quote = FALSE;
	for (chr = 0; prog->line[line][chr] != 0; chr++)
	{
	    if (prog->line[line][chr] == '\"')
		in_quote = (in_quote) ? FALSE : TRUE;
	    if (in_quote) continue;
	    if (prog->line[line][chr] == '(')
		count++;
	    else if (prog->line[line][chr] == ')')
		count--;
	    if (prog->line[line][chr] == '$')
	    {
		chr++;
		if (prog->line[line][chr] != 'n'
		    && prog->line[line][chr] != 'e'
		    && prog->line[line][chr] != 'm'
		    && prog->line[line][chr] != 's'
		    && prog->line[line][chr] != 'N'
		    && prog->line[line][chr] != 'E'
		    && prog->line[line][chr] != 'M'
		    && prog->line[line][chr] != 'S'
		    && prog->line[line][chr] != '0'
		    && prog->line[line][chr] != '1'
		    && prog->line[line][chr] != '2'
		    && prog->line[line][chr] != '3'
		    && prog->line[line][chr] != '4'
		    && prog->line[line][chr] != '5'
		    && prog->line[line][chr] != '6'
		    && prog->line[line][chr] != '7'
		    && prog->line[line][chr] != '$')
		{
		    stcf(ch, "Invalid $ substitution of %c in line %d.\n\r",
			prog->line[line][chr], line);
		    prog->valid = FALSE;
		    myerc += 1;
		}
	    }
	}
	if (in_quote)
	{
	    stcf(ch, "Mismatched \"'s in line %d.\n\r", line);
	    prog->valid = FALSE;
	    myerc += 1;
	}
	if (count < 0)
	{
	    stcf(ch, "%d ( needed in line %d.\n\r", -count, line);
	    prog->valid = FALSE;
	    myerc += 1;
	}
	if (count > 0)
	{
	    stcf(ch, "%d ) needed in line %d.\n\r", count, line);
	    prog->valid = FALSE;
	    myerc += 1;
	}
	if (myerc == 0)
	{
	    for (chr = 0; prog->line[line][chr] != 0; chr++)
	    {
		if (prog->line[line][chr] == 'F' && prog->line[line][chr+1] == 'N')
		{
		    int f, fc = 0;
		    bool found = FALSE;
		    char fnbuf[1000];
		    chr += 2;
		    while (prog->line[line][chr] != 0 && prog->line[line][chr] != '(')
			fnbuf[fc++] = prog->line[line][chr++];
		    if (prog->line[line][chr] == 0)
		    {
			stcf(ch, "Invalid function call in line %d.\n\r", line);
			myerc += 1;
			prog->valid = FALSE;
		    }
		    else
		    {
			fnbuf[fc] = 0;
			for (f = 0; mp_fn[f].proc != NULL; f++)
			{
			    if (!str_cmp(mp_fn[f].name, fnbuf))
				found = TRUE;
			}
			if (!found)
			{
			    stcf(ch, "Invalid function '%s' in line %d.\n\r", fnbuf, line);
			    myerc += 1;
			    prog->valid = FALSE;
			}
		    }
		}
	    }
	}
	if (myerc >= 20) break;
    }

    if (prog->valid != TRUE || myerc > 0)
    {
	stcf(ch, "Compilation failed, with %d errors.\n\r", myerc);
	prog->valid = FALSE;
    }
    else
	stcf(ch, "Compilation sucessful.\n\rHave a cigar.\n\r");
}

void terminate_prog( PROG_DATA *prog, bool mobalive )
{
    PROG_DATA *prg;

    if (mobalive)
	prog->ch->run_trig[prog->trigger] = FALSE;

    if (prog_list == prog)
	prog_list = prog->next;
    else
    {
	for (prg = prog_list; prg != NULL; prg = prg->next)
	{
	    if (prg->next == prog)
		prg->next = prog->next;
	}
    }

    free_mem( prog, sizeof(*prog) );
}

void run_prog( PROG_DATA *prog )
{
    int count, i, j, k, brklevel = 0;
    char buf[MAX_INPUT_LENGTH];  /* command buffer */
    char buf1[MAX_INPUT_LENGTH]; /* Use buf1 and buf2 for arg passing */
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_INPUT_LENGTH]; /* command buffer */
    char *pc, *tmp;
    char com[MAX_STRING_LENGTH];
    char retbuf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    bool found = FALSE;

    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };

    /* VERY VERY SLOPPY WORKAROUND, but misc. crashes forced me */

    for (ch = char_list; ch != NULL; ch = ch->next)
    {
	if (ch == prog->ch)
	{
	    found = TRUE;
	    break;
	}
    }

    if (!found)
    {
	bug("ROGUE PROGRAM CAUGHT.  TERMINATING SAFELY",0);
	terminate_prog( prog, FALSE );
	return;
    }

    if ( prog->running->valid == FALSE )
    {
	terminate_prog( prog, TRUE );
	return;
    }

    prog->frozen = 0;

    for ( count = 0; count < 100; count++)
    {    
	/* get command line */
	if (prog->cur_line == MAX_MOB_PROG_LINES
	    || prog->running->line[prog->cur_line] == NULL)
	{
	    terminate_prog( prog, TRUE );
	    return;
	}
	split_misc(prog->running->line[prog->cur_line++], buf, '|');

	/* substitute alias's and FN's */

	buf1[0] = '$';
	buf1[1] = '\0';
	j = 0;
	for (i = 0; buf[i]!='\0'; i++)
	{
	    if (buf[i]=='$')
	    {
		switch( buf[++i] )
                {
                case 'n' : tmp = GET_NAME(prog->trig); break;
                case 'e' : tmp = he_she  [URANGE(0,prog->trig->sex,2)]; break;
                case 'm' : tmp = him_her [URANGE(0,prog->trig->sex,2)]; break;
                case 's' : tmp = his_her [URANGE(0,prog->trig->sex,2)]; break;
                case 'N' : tmp = GET_NAME(prog->ch); break;
                case 'E' : tmp = he_she  [URANGE(0,prog->ch->sex,2)]; break;
                case 'M' : tmp = him_her [URANGE(0,prog->ch->sex,2)]; break;
                case 'S' : tmp = his_her [URANGE(0,prog->ch->sex,2)]; break;
                case '0' : tmp = prog->r[0]; break;
                case '1' : tmp = prog->r[1]; break;
                case '2' : tmp = prog->r[2]; break;
                case '3' : tmp = prog->r[3]; break;
                case '4' : tmp = prog->r[4]; break;
                case '5' : tmp = prog->r[5]; break;
                case '6' : tmp = prog->r[6]; break;
                case '7' : tmp = prog->r[7]; break;
		default  : tmp = buf1; break;
                }
		smash_misc( tmp, '\"', '\'');
                for (k = 0; k < strlen(tmp); k++)
                    buf3[j++] = tmp[k];
            }
            else
                buf3[j++] = buf[i];
        }
	buf3[j] = '\0';

	j = 0;
	for (i = 0; buf3[i]!='\0'; i++) 
	{
	    if (buf3[i]=='F' && buf3[i+1]=='N')
	    {
		k = 0;
		for (i += 2; buf3[i] != '('; i++)
		    buf1[k++] = buf3[i];
		buf1[k] = '\0';
		k = 0;
		brklevel = 1;
		for (i += 1 ; brklevel > 0; i++)
		{
		    if (buf3[i] == ')') brklevel -= 1;
		    if (buf3[i] == '(') brklevel += 1;
		    if (buf3[i] == '\"')
		    {
			for (i += 1; buf3[i] != '\"'; i++)
			    buf2[k++] = buf3[i];
		    }
		   else  buf2[k++] = buf3[i];
		}
		buf2[--k] = '\0';
		crunch_fn(prog, buf1, buf2, retbuf);
		for (k = 0; retbuf[k] != '\0'; k++)
		    buf[j++] = retbuf[k];
		i -= 1;
	    }
	    else
		buf[j++] = buf3[i];
	}
	buf[j] = '\0';


	pc = one_argument(buf, com);
	if (com[0] == '\0')
	    continue;

	if (!str_prefix("endif",com))
	{
	    if (prog->iflevel > 0) prog->iflevel -= 1;
	    continue;
	}

	if (!str_prefix("else", com))
	{
	    if (prog->iflevel < 2)
		prog->iflevel = (prog->iflevel == 0) ? 1 : 0;
	    continue;
	}

	if (!str_prefix("bug", com))
	{
	    bug(pc, 0);
	    continue;
	}

	/* handle if's and whiles */

	if (!str_prefix("if", com))
	{
	    bool tru = TRUE, not = FALSE, loop = TRUE;
	    if (prog->iflevel > 1)
	    {
		prog->iflevel += 1;
		continue;
	    }
	    while (loop)
	    {
		pc = one_argument(pc, buf3);
		loop = FALSE;
		if (!str_cmp(buf3, "not"))
		{
		    not = TRUE;
		    loop = TRUE;
		    continue;
		}

		if (is_number(buf3))
		{
		    if (atoi(buf3) == 0)
			tru = FALSE;
		}
		else if (!str_cmp(buf3, "FALSE"))
		    tru = FALSE;
	    }
	    if (not) tru = !tru;
	    if (!tru)
		prog->iflevel += 1;

	    continue;
	}

	if (prog->iflevel > 0)
	    continue;

	if (!str_prefix("while", com))
	{
	    bool tru = TRUE, not = FALSE, loop = TRUE;
	    int wl = 0;

	    while (loop)
	    {
		pc = one_argument(pc, buf3);
		loop = FALSE;
		if (!str_cmp(buf3, "not"))
		{
		    not = TRUE;
		    loop = TRUE;
		    continue;
		}
		if (is_number(buf3))
		{
		    if (atoi(buf3) == 0)
			tru = FALSE;
		}
		else if (!str_cmp(buf3, "FALSE"))
		    tru = FALSE;
	    }
	    if (not) tru = !tru;
	    if (!tru)
	    {
		wl = 1;
		while (wl > 0)
		{
		    split_misc(prog->running->line[prog->cur_line++], buf,'|');
		    if (!str_prefix("while", buf))
			wl += 1;
		    else if (!str_prefix("endwhile", buf))
			wl -= 1;
		}
	    }
	    else
	    {
		prog->whilestack[prog->whilelevel++] = prog->cur_line - 1;
	    }
	    continue;
	}

	if (!str_prefix("endwhile", com))
	{
	    if (prog->whilelevel > 0)
		prog->cur_line = prog->whilestack[--prog->whilelevel];
	    continue;
	}
	    
	/* handle writing to variables */

	if ((com[0] == 'R' || com[0] =='r')
		&& isdigit(com[1]) && com[2] == '\0'
		&& pc[0] == '=' && pc[1] == ' ')
	{
	    pc = one_argument(pc, buf3);
	    i = com[1] - '0';
	    if (i < 0 || i > 7)
	    {
		bug("MOB_PROGS: register out of range",0);
		continue;
	    }
	    strcpy(prog->r[i], pc);
	}

	/* handle breaks */

	if (!str_prefix("break",com))
	{
	    terminate_prog( prog, TRUE );
	    return;
	}
	
	/* handle pause */
	if (!str_prefix("pause",com))
	{
	    if (is_number(pc))
		prog->pause = atoi(pc);
	    else
		prog->pause = 0;
	    return;
	}

	if (!str_prefix("freeze", com))
	{
	    if (is_number(pc))
		prog->frozen = 1000 - atoi(pc);
	    else
		prog->frozen = 1;
	    return;
	}

	interpret( prog->ch, buf );

    }

    prog->pause = 0;
}

void crunch_fn(PROG_DATA *prog, char *fn, char *args, char *retbuf)
{
    char ret[MAX_INPUT_LENGTH];
    char pass[MAX_INPUT_LENGTH];
    char subfn[MAX_INPUT_LENGTH];
    char subfnargs[MAX_INPUT_LENGTH];
    int i, j, k, brklevel = 0;
    bool found = FALSE;

    ret[0] = '\0';
    pass[0] = '\0';
    subfnargs[0] = '\0';
    subfn[0] = '\0';
    retbuf[0] = '\0';


    /* sort out any nasty recusive bits */
    j = 0;
    for (i = 0; args[i] != '\0'; i++)
    {
	if (args[i]=='F' && args[i+1]=='N')
	{
	    k = 0;
	    for (i += 2; args[i] != '('; i++)
		subfn[k++] = args[i];
	    subfn[k] = '\0';
	    k = 0;
	    brklevel = 1;
	    for (i += 1 ; brklevel > 0; i++)
	    {
	    	if (args[i] == ')') brklevel -= 1;
		if (args[i] == '(') brklevel += 1;
	        if (args[i] == '\"')
	        {
		    for (i += 1; args[i] != '\"'; i++)
			subfnargs[k++] = args[i];
		}
		else subfnargs[k++] = args[i];
	    }
	    subfnargs[--k] = '\0';
	    crunch_fn(prog, subfn, subfnargs, ret);
	    for (k = 0; ret[k] != '\0'; k++)
		pass[j++] = ret[k];
	    i-=1;
	}
	else
	    pass[j++] = args[i];
    }
    pass[j] = '\0';
    for (i = 0; mp_fn[i].proc != NULL; i++)
    {
	if (!str_cmp(mp_fn[i].name, fn))
	{
	    (*mp_fn[i].proc) (prog, pass, retbuf);
	    found = TRUE;
	    break;
	}
    }
    if (!found)
    {
	bug("Function not found:",0);
	bug(fn, 0);
	strcpy(retbuf, "FALSE");
    }
}

void retn(char *buf, bool state)
{
    if (state)
	strcpy(buf, "TRUE");
    else
	strcpy(buf, "FALSE");
}

void clip_quotes(char *buf)
{
    int pos = strlen(buf) - 1;

    if (buf[pos] == '\"')
	buf[pos] = '\0';
}

void close_progs(CHAR_DATA *ch)
{
    PROG_DATA *pr, *prnext;

    if (prog_list == NULL || ch == NULL)
	return;

    for (pr = prog_list; pr != NULL; pr = prnext)
    {
	prnext = pr->next;
	if (pr->ch == ch)
	    terminate_prog( pr, TRUE );
    }
}

/*************************************************************                
* FN's for the progs to call				     *
*************************************************************/

void mp_str_prefix(PROG_DATA *prog, char *argument, char *ret)
{
    char buf[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, buf);

    if (str_prefix(buf, argument))
	sprintf(ret, "FALSE");
    else sprintf(ret, "TRUE");
}

void mp_pass_var(PROG_DATA *prog, char *argument, char *ret)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0;
    MOB_PROG_DATA *mp;
    PROG_DATA *pr;
    bool ok = FALSE;

    argument = split_semicolon(argument, buf);

    if (isdigit(buf[0]))
	i = buf[0] - '0';

    argument = split_semicolon(argument, buf);

    mp = get_prog_index(argument);

    for (pr = prog_list; pr != NULL; pr = pr->next)
    {
	if (pr->ch == prog->ch && pr->running == mp)
	{
	    strcpy(pr->r[i], buf);
	    ok = TRUE;
	}
    }

    retn(ret, ok);
}

void mp_male_female(PROG_DATA *prog, char *argument, char *ret)
{
    CHAR_DATA *ch;
    char buf1[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, buf1);

    if ((ch = get_char_room(prog->ch, buf1))==NULL
	|| ch->sex == SEX_FEMALE)
    {
    	argument = split_semicolon(argument, buf1);
    	split_semicolon(argument, ret);
	return;
    }

    split_semicolon(argument, ret);
}

void mp_mob_here(PROG_DATA *prog, char *argument, char *ret)
{
    if (get_char_room(prog->ch, argument) == NULL)
	retn(ret, FALSE);
    else
	retn(ret, TRUE);
}

void mp_unfreeze(PROG_DATA *prog, char *argument, char *ret)
{
    char arg[MAX_INPUT_LENGTH];
    PROG_DATA *pr; 
    int i = 0;
    bool ok = FALSE;

    argument = split_semicolon(argument, arg);
    
    if (is_number(arg))
	i = atoi(arg);

    for (pr = prog_list; pr != NULL; pr = pr->next)
    {
	if (!str_cmp(pr->r[i], argument) && pr->frozen > 1)
	{
	    pr->frozen = 0;
	    ok = TRUE;
	}
    }

    retn(ret, ok);
}

void mp_running(PROG_DATA *prog, char *argument, char *ret)
{
    MOB_PROG_DATA *mp;
    PROG_DATA *pr;
    bool found = FALSE;

    mp = get_prog_index(argument);

    for (pr = prog_list; pr != NULL; pr = pr->next)
    {
	if (pr->ch == prog->ch && pr->running == mp)
	    found = TRUE;
    }

    retn(ret, found);
}

void mp_pc_marrying(PROG_DATA *prog, char *argument, char *ret)
{
    if (prog->trig->married_to == NULL)
    {
	retn(ret, FALSE);
	return;
    }

    strcpy(ret, prog->trig->married_to);
}

void mp_spawn(PROG_DATA *prog, char *argument, char *ret)
{
    MOB_PROG_DATA *mp;

    if ((mp = get_prog_index(argument)) == NULL)
    {
	retn(ret, FALSE);
	bug(argument,0);
	return;
    }

    init_prog(prog->ch, prog->trig, prog->r[7], get_prog_index(argument), prog->trigger);
    retn(ret, TRUE);
}

void mp_str_cmp(PROG_DATA *prog, char *argument, char *ret)
{
    char buf[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, buf);

    if (str_cmp(buf, argument))
	sprintf(ret, "FALSE");
    else sprintf(ret, "TRUE");
}

void mp_random(PROG_DATA *prog, char *argument, char *ret)
{
    int num = 1;
    char buf[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, buf);
    if (is_number(buf)) num = atoi(buf);

    split_number_misc(argument, ret, ';', dice(1,num));
}

void mp_multi_check(PROG_DATA *prog, char *argument, char *ret)
{
    char arg1[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, arg1);
    clip_quotes(argument);
    retn(ret, multi_check(arg1, argument));
}

void mp_arg_check(PROG_DATA *prog, char *argument, char *ret)
{
    char arg1[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, arg1);
    clip_quotes(argument);
    retn(ret, arg_check(arg1, argument));
}

void mp_curr_stance(PROG_DATA *prog, char *argument, char *ret)
{
    CHAR_DATA *ch;

    if ((ch = get_char_room(prog->ch, argument)) == NULL
	|| ch->stance == 0)
    {
	retn(ret, FALSE);
	return;
    }

    strcpy(ret, stances[ch->stance].name);
}

void mp_curr_hp(PROG_DATA *prog, char *argument, char *ret)
{
    CHAR_DATA *ch;

    if ((ch = get_char_room(prog->ch, argument)) == NULL)
    {
	strcpy(ret, "0");
	return;
    }

    sprintf(ret, "%d", ch->hit);
}

void mp_release_trigger(PROG_DATA *prog, char *argument, char *ret)
{
    prog->ch->run_trig[prog->trigger] = FALSE;
    retn(ret, TRUE);
}

void mp_claim_trigger(PROG_DATA *prog, char *argument, char *ret)
{
    retn(ret, prog->ch->run_trig[prog->trigger]);
    prog->ch->run_trig[prog->trigger] = TRUE;
}

void mp_company(PROG_DATA *prog, char *argument, char *ret)
{
    retn(ret, (prog->ch->in_room->people != NULL));
}

void mp_dice(PROG_DATA *prog, char *argument, char *ret)
{
    int arg1, arg2;
    char buf[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, buf);
    if (!is_number(buf))
    {
	bug("mp_eval: arg 1 is not a number",0);
	strcpy(ret, "0");
	return;
    }
    arg1 = atoi(buf);
    argument = split_semicolon(argument, buf);
    if (!is_number(buf))
    {
	bug("mp_eval: arg 2 is not a number",0);
	strcpy(ret, "0");
	return;
    }
    arg2 = atoi(buf);

    sprintf(ret, "%d", dice(arg1, arg2));  
}    

void mp_eval(PROG_DATA *prog, char *argument, char *ret)
{
    int arg1, arg2;
    char oparand[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    argument = split_semicolon(argument, buf);
    if (!is_number(buf))
    {
	bug("mp_eval: arg 1 is not a number",0);
	retn(ret, FALSE);
	return;
    }
    arg1 = atoi(buf);
    argument = split_semicolon(argument, oparand);
    argument = split_semicolon(argument, buf);
    if (!is_number(buf))
    {
	bug("mp_eval: arg 2 is not a number",0);
	retn(ret, FALSE);
	return;
    }
    arg2 = atoi(buf);

    if (!str_cmp(oparand, "=="))
    {
	retn(ret, (arg1 == arg2) ? TRUE : FALSE);
	return;
    }
    else if (!str_cmp(oparand, "!="))
    {
	retn(ret, (arg1 != arg2));
	return;
    }
    else if (!str_cmp(oparand, "<"))
    {
	retn(ret, (arg1 < arg2));
	return;
    }
    else if (!str_cmp(oparand, ">"))
    {
	retn(ret, (arg1 > arg2) ? TRUE : FALSE);
	return;
    }
    else if (!str_cmp(oparand, "<="))
    {
	retn(ret, (arg1 <= arg2));
	return;
    }
    else if (!str_cmp(oparand, ">="))
    {
	retn(ret, (arg1 >= arg2));
	return;
    }
    else if (!str_cmp(oparand, "<>"))
    {
	retn(ret, (arg1 != arg2));
	return;
    }
    else
    {
	sprintf(buf, "mp_eval:  '%s' is not a valid operator", oparand);
	bug(buf, 0);
	retn(ret, FALSE);
    }
}

