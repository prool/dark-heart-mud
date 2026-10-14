/******************************************************************
 * Stat_Save-- (C) 1996, by Chris Mellish                         *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "recycle.h"

void load_league_table args( ( ) );
void save_league_table args( ( ) );
void move_player args ( (int i, int j) );
void swap_player args ( (int i, int j) );
void save_html_league args( ( ) );
void load_balance_table args( ( ) );
void save_balance_table args( ( ) );
void sort_balance args( ( ) );
 
void initialise_site_list()
{
    FILE *fp;

    site_list = NULL;
    return;

    if ((fp = fopen("site.list","r"))==NULL)
	return;

    for ( ; ; )
    {
	SITE_DATA *s;
	char letter;

	if ( (letter = fread_letter( fp ) ) == '~' )
	    break;

	s = alloc_perm( sizeof(*s) );

	s->number = fread_string( fp );
	s->name = fread_string( fp );
	s->next = site_list;
	site_list = s;
    }

    fclose( fp );
}

void initialise_email_list()
{
    FILE *fp;

    email_list = NULL;

    if ((fp = fopen("email.list","r"))==NULL)
	return;

    for ( ; ; )
    {
	EMAIL_DATA *s;
	char letter;

	if ( (letter = fread_letter( fp ) ) == '~' )
	    break;

	s = alloc_perm( sizeof(*s) );

	s->address = fread_string( fp );
	s->next = email_list;
	email_list = s;
    }

    fclose( fp );
}

void add_email_to_list(char *address)
{
    EMAIL_DATA *s;
    FILE *fp;

    if (address == NULL
	|| !str_cmp(address, "END"))
	return;

    s = alloc_perm( sizeof(*s) );

    s->address   = str_dup(address);
    s->next   = email_list;
    email_list = s;

    if ((fp = fopen("email.list","w"))==NULL)
    {
	bug("Error saving email.list!",0);
	return;
    }

    for (s = email_list; s != NULL; s = s->next)
	fprintf(fp, "-%s~\n", s->address);
    fprintf(fp, "~");

    fclose( fp );
}

bool check_email(char *address)
{
    EMAIL_DATA *em;

    for (em = email_list; em != NULL; em = em->next)
	if (!str_cmp(em->address, address)) return TRUE;

    return FALSE;
}

void add_site_to_list(char *num, char *name)
{
    SITE_DATA *s;
    FILE *fp;
   return;

    if (num == NULL || name == NULL
	|| !str_cmp(num, "END") || !str_cmp(name, "END"))
	return;

    s = alloc_perm( sizeof(*s) );

    s->number = str_dup(num);
    s->name   = str_dup(name);
    s->next   = site_list;
    site_list = s;

    if ((fp = fopen("site.list","w"))==NULL)
    {
	bug("Error saving site.list!",0);
	return;
    }

    for (s = site_list; s != NULL; s = s->next)
    {
	fprintf(fp, "-%s~ %s~\n", s->number, s->name);
    }
    fprintf(fp, "~");

    fclose( fp );
}

char * get_site_old(char *num)
{
    SITE_DATA *s;
    bool found = FALSE;

    for (s = site_list; s != NULL; s = s->next)
    {
	if (str_cmp(s->number, num))
	    continue;

	found = TRUE;
	break;
    }

    if (found)
	return s->name;

    return NULL;
}

void initialise_league_table()
{
    int i;

    for (i = 0; i < MAX_LEAGUE; i++)
    {
	league_table[i].name[0] = '\0';
	league_table[i].pkills = 0;
	league_table[i].pkilled = 0;
	league_table[i].bmk = 0;
    }

    load_league_table();
}

void initialise_balance_table()  
{
    int i;
    for (i = 0; i < 7; i++)
    {
        balance_info[i].pkills = 0;
        balance_info[i].pkilled = 0;
        balance_info[i].members = 0;
        balance_info[i].total_score = 0;
        balance_info[i].position = 0;
        balance_info[i].total_gen = 0;
        balance_info[i].num_tertiary = 0;
        balance_info[i].total_alignment = 0;
    }

    load_balance_table();  
}

void load_balance_table( )  
{
    FILE *fp;
    int i;

    if ((fp = fopen("balance.table","r")) == NULL)
    {
        bug("Creating balance.table",0);
        save_balance_table();
        return;
    }

    for (i = 0; i < 7; i++)
    {
	char letter;
        fread_to_eol( fp );
        fread_to_eol( fp );
        fread_to_eol( fp );
	letter = fread_letter( fp );
        balance_info[i].pkills = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].pkilled = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].members = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].total_score = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].position = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].total_gen = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].num_tertiary = fread_number( fp ); fread_to_eol(fp);
	letter = fread_letter( fp );
        balance_info[i].total_alignment = fread_number( fp ); fread_to_eol(fp);
    }

    sort_balance();

    fclose( fp );  
}

void save_balance_table()
{
    FILE *fp;
    int i;

    if ((fp = fopen("balance.table","w")) == NULL)
    {
        bug("Error Creating balance.table",0);
	exit( 1 );
    }

    for (i = 0; i < 7; i++)
    {
	fprintf(fp, "*******************************************\n");
	fprintf(fp, "*             S E C T I O N   %d           *\n", i);
	fprintf(fp, "*******************************************\n");
	fprintf(fp, "*        %d          * Pkills              *\n", balance_info[i].pkills) ;
	fprintf(fp, "*        %d          * Pkilled             *\n", balance_info[i].pkilled);
	fprintf(fp, "*        %d          * Members             *\n", balance_info[i].members);
	fprintf(fp, "*        %d          * Total Score         *\n", balance_info[i].total_score);
	fprintf(fp, "*        %d          * Position            *\n", balance_info[i].position);
	fprintf(fp, "*        %d          * Total Generation    *\n", balance_info[i].total_gen);
	fprintf(fp, "*        %d          * Number of Tertiarys *\n", balance_info[i].num_tertiary);
	fprintf(fp, "*        %d          * Total Alignment     *\n",balance_info[i].total_alignment);
    }

	fprintf(fp, "******************************************\n");

    fclose( fp );
}

void sort_balance()
{
    int i;

    for (i = 0; i < 7; i++)
    {
	balance_info[i].position = i + 1;
    }	    
}

void do_configure(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char *wholearg;
    CHAR_DATA *vch;
    int ox, oy;

    argument = one_argument( argument, arg1 );
    wholearg = argument;
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if (arg1[0] == '\0')
    {
	send_to_char("Current Configuration: \n\r",ch);

	sprintf(buf, "hitpoints = Mobile Hp     *= %d / %d\n\r",
	    config_info->mob_hp_mult, config_info->mob_hp_div);
	send_to_char(buf, ch);
	sprintf(buf, "level     = Mobile Level  *= %d / %d\n\r",
	    config_info->mob_level_mult, config_info->mob_level_div);
	send_to_char(buf, ch);
    	sprintf(buf, "damage    = Mobile Damage *= %d / %d\n\r\n",
	    config_info->mob_dam_mult, config_info->mob_dam_div);
    	send_to_char(buf, ch);
    	sprintf(buf, "rexp     = Research Exp Cost *= %d / %d\n\r",
	    config_info->research_exp_mult, config_info->research_exp_div);
    	send_to_char(buf, ch);
    	sprintf(buf, "rtime    = Research Time Cost *= %d / %d\n\r",
	    config_info->research_time_mult, config_info->research_time_div);
    	send_to_char(buf, ch);
    	sprintf(buf, "prac     = Prac Exp Cost      *= %d / %d\n\r",
	    config_info->prac_exp_mult, config_info->prac_exp_div);
    	send_to_char(buf, ch);
    	sprintf(buf, "train    = Train Exp Cost     *= %d / %d\n\r",
	    config_info->train_exp_mult, config_info->train_exp_div);
    	send_to_char(buf, ch);
	sprintf(buf, "autohelp = Automatic Help Prompt = %s\n\r",config_info->autohelp);
	send_to_char(buf, ch);
	return;
    }

    if (!str_prefix(arg1, "autohelp"))
    {
	if (wholearg[0] == '\0')
	{
	    send_to_char("Syntax:  configure autohelp <help page>",ch);
	    return;
	}

	free_string( config_info->autohelp );
	config_info->autohelp = str_dup( wholearg );
	send_to_char("Successfully updated Auto Help.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }	

    if (!is_number(arg2) || !is_number(arg3))
    {
	send_to_char("Syntax:  configure <variable> <x> <y>\n\r",ch);
	send_to_char("where x:y is the ratio you wish to set the variable to.\n\r",ch);
	send_to_char("eg. configure hitpoints 2 1\n\r",ch);
	send_to_char("Doubles the number of hp every mob has.\n\r",ch);
	send_to_char("Configure on it's own lists all variables.\n\r",ch);
	return;
    }

    if (atoi(arg3) == 0 || atoi(arg2)==0)
    {
	send_to_char("Values of zero are not allowed.\n\r",ch);
	return;
    }

    if (atoi(arg2) / atoi(arg3) > 4)
    {
	send_to_char("Ratio too large.\n\r",ch);
	return;
    }

    if (atoi(arg3) / atoi(arg2) > 4)
    {
	send_to_char("Ratio too small.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "hitpoints"))
    {
	ox = config_info->mob_hp_mult;
	oy = config_info->mob_hp_div;

	config_info->mob_hp_mult = atoi(arg2);
	config_info->mob_hp_div  = atoi(arg3);

	for (vch = char_list; vch != NULL; vch = vch->next)
	{
	    if (IS_NPC(vch))
	    {
		vch->hit = vch->hit * oy / ox;
		vch->hit = vch->hit * config_info->mob_hp_mult / config_info->mob_hp_div;
		vch->max_hit = vch->max_hit * oy / ox;
		vch->max_hit = vch->max_hit * config_info->mob_hp_mult / config_info->mob_hp_div;
	    }
	}

	send_to_char("Successful update of mobile hitpoints.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "damage"))
    {
	int i;

	ox = config_info->mob_dam_mult;
	oy = config_info->mob_dam_div;

	config_info->mob_dam_mult = atoi(arg2);
	config_info->mob_dam_div  = atoi(arg3);

	for (vch = char_list; vch != NULL; vch = vch->next)
	{
	    if (IS_NPC(vch))
	    {
		for (i = 0; i < 3; i++)
		{
		vch->damage[i] = vch->damage[i] * oy / ox;
		vch->damage[i] = vch->damage[i] * config_info->mob_hp_mult / config_info->mob_hp_div;
		}
	    }
	}

	send_to_char("Successful update of mobile damage.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "level"))
    {
	ox = config_info->mob_level_mult;
	oy = config_info->mob_level_div;

	config_info->mob_level_mult = atoi(arg2);
	config_info->mob_level_div  = atoi(arg3);

	for (vch = char_list; vch != NULL; vch = vch->next)
	{
	    if (IS_NPC(vch))
	    {
		vch->level = vch->level * oy / ox;
		vch->level = vch->level * config_info->mob_hp_mult /config_info->mob_hp_div;
	    }
	}

	send_to_char("Successful update of mobile level.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "rtime"))
    {
	config_info->research_time_mult = atoi(arg2);
	config_info->research_time_div  = atoi(arg3);

	send_to_char("Successful update of research time.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "rexp"))
    {
	config_info->research_exp_mult = atoi(arg2);
	config_info->research_exp_div  = atoi(arg3);

	send_to_char("Successful update of research exp.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "prac"))
    {
	config_info->prac_exp_mult = atoi(arg2);
	config_info->prac_exp_div  = atoi(arg3);

	send_to_char("Successful update of practice exp.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }

    if (!str_prefix(arg1, "train"))
    {
	config_info->train_exp_mult = atoi(arg2);
	config_info->train_exp_div  = atoi(arg3);

	send_to_char("Successful update of train exp.\n\r",ch);
	save_config();
	send_to_char("Parameters successfully saved in config.sys.\n\r",ch);
	return;
    }




    send_to_char("Cannot update a non-existant variable.\n\r",ch);
    send_to_char("config with no arguments to list.\n\r",ch);

}

void initialise_config()
{
    if (config_info == NULL)
	config_info = alloc_perm( sizeof(*config_info) );

    config_info->mob_hp_mult	= 1;
    config_info->mob_hp_div	= 1;
    config_info->mob_level_mult	= 1;
    config_info->mob_level_div	= 1;
    config_info->mob_dam_mult	= 1;
    config_info->mob_dam_div	= 1;
    config_info->prac_exp_mult	= 1;
    config_info->prac_exp_div	= 1;
    config_info->train_exp_mult	= 1;
    config_info->train_exp_div	= 1;
    config_info->research_exp_mult	= 1;
    config_info->research_exp_div	= 1;
    config_info->research_time_mult	= 1;
    config_info->research_time_div	= 1;
    config_info->max_players		= 0;

    load_config();
}

void load_config()
{
    FILE *fp;

    if ((fp = fopen("config.sys","r")) == NULL)
    {
	bug("Creating config.sys",0);
	save_config();
	return;
    }

    fread_to_eol( fp );

    config_info->mob_hp_mult	= fread_number( fp ); fread_to_eol( fp );
    config_info->mob_hp_div	= fread_number( fp ); fread_to_eol( fp );
    config_info->mob_level_mult	= fread_number( fp ); fread_to_eol( fp );
    config_info->mob_level_div	= fread_number( fp ); fread_to_eol( fp );
    config_info->mob_dam_mult	= fread_number( fp ); fread_to_eol( fp );
    config_info->mob_dam_div	= fread_number( fp ); fread_to_eol( fp );
    config_info->prac_exp_mult	= fread_number( fp ); fread_to_eol( fp );
    config_info->prac_exp_div	= fread_number( fp ); fread_to_eol( fp );
    config_info->train_exp_mult	= fread_number( fp ); fread_to_eol( fp );
    config_info->train_exp_div	= fread_number( fp ); fread_to_eol( fp );
    config_info->research_exp_mult = fread_number( fp ); fread_to_eol( fp );
    config_info->research_exp_div = fread_number( fp ); fread_to_eol( fp );
    config_info->research_time_mult = fread_number( fp ); fread_to_eol( fp );
    config_info->research_time_div = fread_number( fp ); fread_to_eol( fp );
    config_info->autohelp = fread_string( fp ); fread_to_eol( fp );
    config_info->max_players	= fread_number( fp ); fread_to_eol( fp );
    fclose( fp );
}

void save_hits()
{
    FILE *fp;

    if ((fp = fopen("hits.data","w")) == NULL)
    {
	bug("Error saving to hits.data",0);
	return;
    }

    fprintf(fp, "%d %d", highhits, hits);
    fclose( fp );
}

void load_hits()
{
    FILE *fp;

    if ((fp = fopen("hits.data","r")) == NULL)
    {
	save_hits();
	return;
    }

    highhits = fread_number( fp );
    hits = fread_number( fp );

    fclose( fp );
}


void save_config()
{
    FILE *fp;

    if ((fp = fopen("config.sys","w")) == NULL)
    {
	bug("Error saving to config.sys",0);
	return;
    }

    fprintf( fp, "Config.sys - Configuration file for BTL code\n");
    fprintf( fp, "%d   = Mobile Hp Multiplier\n", 	config_info->mob_hp_mult);
    fprintf( fp, "%d   = Mobile Hp Divider\n", 		config_info->mob_hp_div);
    fprintf( fp, "%d   = Mobile Level Multiplier\n", 	config_info->mob_level_mult);
    fprintf( fp, "%d   = Mobile Level Divider\n", 	config_info->mob_level_div);
    fprintf( fp, "%d   = Mobile Dam Multiplier\n", 	config_info->mob_dam_mult);
    fprintf( fp, "%d   = Mobile Dam Divider\n", 	config_info->mob_dam_div);
    fprintf( fp, "%d   = Prac exp mult\n", 		config_info->prac_exp_mult);
    fprintf( fp, "%d   = Prac exp div\n", 		config_info->prac_exp_div);
    fprintf( fp, "%d   = Train exp mult\n",		config_info->train_exp_mult);
    fprintf( fp, "%d   = Train exp div\n", 		config_info->train_exp_div);
    fprintf( fp, "%d   = Research exp mult\n", 		config_info->research_exp_mult);
    fprintf( fp, "%d   = Research exp div\n",		config_info->research_exp_div);
    fprintf( fp, "%d   = Research time mult\n",		config_info->research_time_mult);
    fprintf( fp, "%d   = Research time div\n",		config_info->research_time_div);
    if (config_info->autohelp != NULL)
    	fprintf( fp, "%s~ = Auto Help\n",		config_info->autohelp);
    else
	fprintf( fp, "how to register~ = Auto Help\n");
    fprintf( fp, "%d   = Max Players ever logged on\n", config_info->max_players);
    fclose( fp );
}

void load_league_table()
{
    FILE *fp;
    int i;

    if ((fp = fopen("league.table","r")) == NULL)
    {
	bug("Creating League Table",0);
	save_league_table();
	return;
    }

    for ( ; ; )
    {
	i = fread_number( fp );
	
	if (i < 0)
	    break;

	league_table[i].pkills = fread_number( fp );
	league_table[i].pkilled = fread_number( fp );
	league_table[i].bmk = fread_number( fp );
	league_table[i].score = fread_number( fp );
	strcpy(league_table[i].name,fread_string( fp ));
    }

    fclose( fp );

/* removed for speed.  if you change the score system, add back in

    for (i = 0; i < MAX_LEAGUE; i++)
    {
	if (league_table[i].name[0] != '\0')
	{
	    bool found = FALSE;
	    CHAR_DATA *vch;

	    vch = alloc_mem(sizeof(*vch));
	    vch = new_char();
	    vch->pcdata = new_pcdata();
	    found = do_load_char_obj(vch, league_table[i].name);
	    if (found && vch->level < 100)
	    {
		check_player_table(vch);
	    	extract_char(vch, TRUE);
	    }
	    else
	    {
		league_table[i].name[0] = '\0';
		league_table[i].pkills = 0;
		league_table[i].pkilled = 0;
		league_table[i].bmk = 0;
		league_table[i].score = 0;
	    }
	}
    }

*/
}
    
void save_league_table()
{
    FILE *fp;
    int i;

    if ((fp = fopen("league.table","w")) == NULL)
    {
	bug("Cannot open league.table for writing",0);
	exit(0);
    }

    for (i = 0; i < MAX_LEAGUE; i++)
    {
	if (league_table[i].name[0] != '\0')
	{
	    fprintf(fp, "%d %d %d %d %d %s~\n\r",
		i,
		league_table[i].pkills,
		league_table[i].pkilled,
		league_table[i].bmk,
		league_table[i].score,
		league_table[i].name);
	}
    }

    fprintf(fp, "-1\n\r");

    fclose( fp );

    save_html_league();
}

int league_score( CHAR_DATA *ch )
{
    int i,score, sn;

    score = 0;
    score += 10 * ch->pkills;
    score -= 15 * ch->pkilled;
    score += ch->best_mob_killed;
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (ch->pcdata->learned[sn] > 70)
	    score += 1;
    }

    score += (ch->pcdata->perm_hit - 100) / 50;
    score += (ch->pcdata->perm_move - 100) / 50;

    if (ch->secondaryrace != 0) score += 10;
    score += 5  * (ch->fire + ch->water + ch->air + ch->earth + ch->entropy + ch->lifemagic);

    for (i = 1; i < MAX_DISCIPLINES; i++)
	score += (ch->power[i] > 0) ? ch->power[i] : 0;

    return score;
}


void check_player_table(CHAR_DATA *ch)
{
    int i,score;
    bool found=FALSE;

    crash("check player table");

    if (IS_NPC(ch) || ch->level > 99 || IS_IMMORTAL(ch) )
	return;

    score = league_score( ch );
	
    if (league_table[MAX_LEAGUE - 1].score > score)
	return;

    for (i = 0; i < MAX_LEAGUE; i++)
    {
	if (!str_cmp(league_table[i].name, ch->pcdata->switchname))
	{
	    league_table[i].pkills = ch->pkills;
	    league_table[i].bmk = ch->best_mob_killed;
	    league_table[i].pkilled = ch->pkilled;
	    league_table[i].score = score;
	    found = TRUE;
	    break;
	}
    }

    if (!found)
    {
	if (score > league_table[MAX_LEAGUE].score)
	{
	    i = MAX_LEAGUE - 1;
	    league_table[MAX_LEAGUE-1].pkills = ch->pkills;
	    league_table[MAX_LEAGUE-1].pkilled = ch->pkilled;
	    league_table[MAX_LEAGUE-1].score = score;
	    league_table[MAX_LEAGUE-1].bmk = ch->best_mob_killed;
	    strcpy(league_table[i].name, ch->pcdata->switchname);
	    found = TRUE;
	}
    }

    if (!found)
	return;

    for ( ; i > 0; i--)
    {
	if (league_table[i].score > league_table[i-1].score)
	    swap_player(i, i-1);
    }

    for (i = 0 ; i < MAX_LEAGUE-1; i++)
    {
	if (league_table[i].score < league_table[i+1].score)
	    swap_player(i, i+1);
    }
    save_league_table();
}

/* move league slot j into i */

void move_player(int i, int j)
{
    strcpy(league_table[i].name, league_table[j].name);
    league_table[i].pkills = league_table[j].pkills;
    league_table[i].pkilled = league_table[j].pkilled;
    league_table[i].score = league_table[j].score;
    league_table[i].bmk = league_table[j].bmk;
}

void swap_player(int i, int j)
{
    move_player(MAX_LEAGUE, i);
    move_player(i, j);
    move_player(j, MAX_LEAGUE);
}

void do_league(CHAR_DATA *ch, char *argument)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    banner2_to_char("LEAGUE TABLE",ch);
    
    for (i = 0; i < MAX_LEAGUE; i++)
    {
	if (league_table[i].name[0] != '\0')
	{
	    sprintf(buf, "%2d. %-15s with a score of %6d, with %d pkills, %d pkilled.\n\r",
		i+1,
		league_table[i].name,
		league_table[i].score,
		league_table[i].pkills,
		league_table[i].pkilled);
	    send_to_char(buf,ch);
	}
    }

    divide2_to_char(ch);
}

void do_total(CHAR_DATA *ch, char *argument)
{
    int score, i, tempscore = 0, sn;
    char buf[MAX_STRING_LENGTH];

    banner2_to_char("YOU HAVE SCORED",ch);

    score = 0;
    score += 10 * ch->pkills;
    sprintf(buf, "%d pkills x 10 = %d points",ch->pkills, score);
    cent_to_char(buf, ch);
    score -= 15 * ch->pkilled;
    sprintf(buf, "%d times pkilled x -15 = %d points", 
	ch->pkilled, ch->pkilled * (-15));
    cent_to_char(buf, ch);
    score += ch->best_mob_killed;
    sprintf(buf, "Best mob level killed %d = %d points",
	ch->best_mob_killed, ch->best_mob_killed);
    cent_to_char(buf, ch);
    tempscore = 0;
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (ch->pcdata->learned[sn] > 70)
	    tempscore += 1;
    }
    score += tempscore;
    sprintf(buf, "Skills learnt bonus = %d points", tempscore);
    cent_to_char(buf, ch);
    score += (ch->pcdata->perm_hit - 100) / 50;
    score += (ch->pcdata->perm_move - 100) / 50;
    sprintf(buf, "Hit Point bonus = %d points", (ch->pcdata->perm_hit -100) / 50);
    cent_to_char(buf, ch);
    sprintf(buf, "Vitality Point bonus = %d points",(ch->pcdata->perm_move - 100)/ 50);
    cent_to_char(buf, ch);

    if (ch->secondaryrace != 0)
    {
	score += 10;
	cent_to_char("Being a secondary race = 10 points",ch);
    }
    score += 5  * (ch->fire + ch->water + ch->air + ch->earth + ch->entropy+ch->lifemagic);
    sprintf(buf, "Spell casting ability bonus of %d points",
	5 * (ch->fire + ch->water + ch->air + ch->earth + ch->entropy + ch->lifemagic));
    cent_to_char(buf, ch);
    tempscore = 0;
    for (i = 1; i < MAX_DISCIPLINES; i++)
	tempscore += (ch->power[i] > 0) ? ch->power[i] : 0;
    score += tempscore;
    sprintf(buf, "Disciplines Trained bonus of %d points", tempscore);
    cent_to_char(buf, ch);
    sprintf(buf, "Total Score: %d points",score);
    banner2_to_char(buf, ch);
    check_player_table(ch);
}

void save_html_league()
{
    FILE *fp;
    int i;

    if ((fp = fopen("../../public_html/league.html","w"))==NULL)
    {
	bug("Cannot save to league.html",0);
	return;
    }

    fprintf(fp, "<HTML><HEAD><TITLE>League Table</TITLE></HEAD>");
    fprintf(fp, "<BODY TEXT=\"FFFFFF\" BACKGROUND=\"backgrounds/granite.gif\">");
    fprintf(fp, "<P ALIGN=Center><H1 ALIGN=Center>League Table</H1>\n\r<HR>\n\r");
    fprintf(fp, "<P ALIGN=Left>");

    for (i = 0; i < MAX_LEAGUE; i++)
    {
        if (league_table[i].name[0] != '\0')
	fprintf(fp, "%2d. %-15s with a score of %6d, with %d pkills, %d pkilled.<P>\n\r",
		i+1,
		league_table[i].name,
		league_table[i].score,
		league_table[i].pkills,
		league_table[i].pkilled);
    }

    fprintf(fp, "<HR>\n\r<P ALIGN=Center>Maintained by BTL - Code by Chris Mellish\n\r");

    fclose( fp );
}

void check_pkill_stat_file()
{
    FILE *fp;

    if ((fp = fopen("../../public_html/pkillers.html","r"))==NULL)
    {
	bug("Creating pkillers.html",0);
    }
    else
    {
	fclose( fp );
	return;
    }

    if ((fp = fopen("../../public_html/pkillers.html","w"))==NULL)
    {
	bug("Error creating pkillers.html",0);
	return;
    }

    fprintf(fp, "<HTML><HEAD><TITLE>PKiller Chronicals</TITLE></HEAD>");
    fprintf(fp, "<BODY TEXT=\"FFFFFF\" BACKGROUND=\"backgrounds/granite.gif\">");
    fprintf(fp, "<P ALIGN=Center><H1 ALIGN=Center>The Player Killers Chronicals</H1>\n\r<HR>\n\r");
    fprintf(fp, "<P ALIGN=Left>\n\r");
    fclose( fp );
}

void add_pkill(CHAR_DATA *ch, CHAR_DATA *victim, bool combat)
{
    FILE *fp;
    int levdiff;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch) || IS_NPC(victim) || ch->level > 99 || ch==victim)
	return;

    check_pkill_stat_file();

    ch->pkills += 1;
    victim->pkilled += 1;

    if ((fp = fopen("../../public_html/pkillers.html","a"))==NULL)
    {
	bug("Error opening pkillers.html",0);
	return;
    }

    fprintf(fp, "<h3>%-15s : %s killed %s</h3>\n\r",
	(char *) ctime( &current_time),
	ch->pcdata->switchname,
	victim->pcdata->switchname);

    switch ( ch->secondaryrace )
    {
	case 0 : strcpy(buf, "mortal");break;
	case 1 : strcpy(buf, "Dragon");break;
	case 2 : strcpy(buf, "Daemon");break;
	case 3 : strcpy(buf, "Vampire");break;
	case 4 : strcpy(buf, "Lupine");break;
	case 5 : strcpy(buf, "Eladrin");break;
	case 6 : strcpy(buf, "Warlock");break;
	case 7 : strcpy(buf, "Hero");break;
    }

    fprintf(fp,"%s, a %s, hp %d, spell skill %d killed ",
	ch->pcdata->switchname,
	buf,
	ch->max_hit,
	ch->fire + ch->air + ch->water + ch->earth + ch->entropy + ch->lifemagic);
	
    switch ( victim->secondaryrace )
    {
	case 0 : strcpy(buf, "mortal");break;
	case 1 : strcpy(buf, "Dragon");break;
	case 2 : strcpy(buf, "Daemon");break;
	case 3 : strcpy(buf, "Vampire");break;
	case 4 : strcpy(buf, "Lupine");break;
	case 5 : strcpy(buf, "Eladrin");break;
	case 6 : strcpy(buf, "Warlock");break;
	case 7 : strcpy(buf, "Hero");break;
    }

    fprintf(fp,"%s, a %s, hp %d, spell skill %d%s<p>\n\r",
	victim->pcdata->switchname,
	buf,
	victim->max_hit,
	victim->fire + victim->air + victim->water + victim->earth +
		victim->entropy,
	combat ? " in combat." : ".");

    if (combat)
    {
	fprintf(fp, "The verdict: ");

    	levdiff = victim->level - ch->level;

    	if (levdiff < -20)
	    fprintf(fp, "Slaughter of Novices");
    	else if (levdiff < -10)
	    fprintf(fp, "An almost fair fight");
    	else if (levdiff < 10)
	    fprintf(fp, "An even match");
    	else if (levdiff < 20)
	    fprintf(fp, "David beats Goliath again");
    	else
	    fprintf(fp, "Incredible");

    	fprintf(fp, ".<p>");
    }

    fprintf(fp, "<p><hr>\n\r");

    fclose( fp );
}
