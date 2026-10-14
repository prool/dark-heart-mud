/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "hell.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif


int rename(const char *oldfname, const char *newfname);

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
	if (IS_SET(flag,1<<count))
	{
	    if (count < 26)
		buf[pos] = 'A' + count;
	    else
		buf[pos] = 'a' + (count - 26);
	    pos++;
	}
    }

    if (pos == 0)
    {
	buf[pos] = '0';
	pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST        100
static  OBJ_DATA *      rgObjNest       [MAX_NEST];



/*
 * Local functions.
 */
void    fwrite_char     args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fwrite_obj      args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void    fwrite_pet      args( ( CHAR_DATA *pet, FILE *fp) );
void    fread_char      args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet       args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_obj       args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;
    char *tmp;

    if (ch == NULL) return;

    crash("save_char_obj place : 1");

    if ( IS_NPC(ch) || ch->registerred == 0)
	return;

    if (ch->registerred == 3) ch->registerred = 0;

    tmp = "";
    tmp = str_dup(ch->name);
    set_pc_name(ch, ch->pcdata->switchname);
    set_switchname(ch, tmp);


    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    crash("save_char_obj place : 2");

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL)
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
	}

	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    crash("save_char_obj place : 3");
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
    tmp = str_dup(ch->name);
    crash("save_char_obj place : 4");
    set_pc_name(ch, ch->pcdata->switchname);
    set_switchname(ch, tmp);
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn,pos;
    char buf[MAX_STRING_LENGTH];
    int i,j;

    crash("frwite char start");

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER" );

    fprintf( fp, "Name %s~\n",  ch->name                );
    fprintf( fp, "SwName %s~\n",ch->pcdata->switchname  );
    fprintf( fp, "Id   %ld\n", ch->id                   );
    fprintf( fp, "LogO %ld\n",  current_time            );
    fprintf( fp, "Vers %d\n",   (ch->version == 0) ? 8 : ch->version);
    if (ch->short_descr[0] != '\0')
	fprintf( fp, "ShD  %s~\n",      ch->short_descr );
    if( ch->long_descr[0] != '\0')
	fprintf( fp, "LnD  %s~\n",      ch->long_descr  );
    if (ch->description[0] != '\0')
	fprintf( fp, "Desc %s~\n",      ch->description );
    if (ch->prompt != NULL || !str_cmp(ch->prompt,"<%hhp %mm %vmv %xexp> "))
	fprintf( fp, "Prom %s~\n",      ch->prompt      );
    if (ch->tempprompt != NULL)
	fprintf( fp, "PromTemp %s~\n",      ch->tempprompt      );
    if (ch->fightprompt != NULL)
	fprintf( fp, "FightPrompt %s~\n",      ch->fightprompt      );
    fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
    if (ch->clan)
	fprintf( fp, "Clan %s~\n",clan_table[ch->clan].name);
    fprintf( fp, "RegKey      %d\n",ch->registration_key);
    fprintf( fp, "RegLogins   %d\n",ch->registration_logins);
    fprintf( fp, "EmailSet    %d\n",ch->email_set	);
    fprintf( fp, "Registerred %d\n",ch->registerred	);
    fprintf( fp, "Altitude   %d\n", ch->altitude	);
    fprintf( fp, "Grant       %ld\n",  ch->grant_cmds  	);
    fprintf( fp, "ImmPower    %d\n",ch->immpower    	);
    fprintf( fp, "BuildRealm  %d\n", ch->build_realm	);
    fprintf( fp, "Breaths     %d\n", ch->breaths	);
    fprintf( fp, "Blood       %d\n", ch->blood		);
    fprintf( fp, "Beast       %d\n", ch->beast		);
    fprintf( fp, "QProg       %ld\n", ch->quest_progress);
    fprintf( fp, "QP          %d\n",   ch->qp           );
    fprintf( fp, "Sex         %d\n",   ch->sex          );
    fprintf( fp, "Souls       %d\n",   ch->pcdata->souls);
    fprintf( fp, "Levl        %d\n",   ch->level        );
    fprintf( fp, "CurLang     %d\n",   ch->curlang      );
    fprintf( fp, "FightStyle  %ld\n", ch->fight_style	);
    fprintf( fp, "DesiredDist %d\n", ch->des_dist	);
    fprintf( fp, "BodyForm    %d\n", ch->body_form      );
    fprintf( fp, "BestMobKilled %d\n", ch->best_mob_killed         );
    fprintf( fp, "Rage %d\n", ch->rage         );
    fprintf( fp, "VoodooPoints %d\n", ch->pcdata->voodoo_points         );
    fprintf( fp, "MobKills %d\n", ch->pcdata->mob_kills         );
    fprintf( fp, "MobKilled %d\n", ch->pcdata->mob_killed         );
    fprintf( fp, "AutoLookLine %d\n", ch->auto_look_line);
    if (ch->pcdata->last_login_from != NULL)
    {
	fprintf( fp, "LastLoginTime %s~\n", ch->pcdata->last_login_time);
	fprintf( fp, "LastLoginFrom %s~\n", ch->pcdata->last_login_from);
    }
    crash("frwite char place 2");

    fprintf( fp, "Fth  %d\n",  ch->faith                );
    fprintf( fp, "FthTrue  %d\n",  ch->true_faith       );
    fprintf( fp, "Sanity  %d\n",  ch->pcdata->sanity    );
    fprintf( fp, "Form %d\n",   ch->secondaryrace       );
    fprintf( fp, "Tertiary %d\n",   ch->tertiaryrace    );    
    fprintf( fp, "Gen  %d\n",   ch->generation          );
//    fprintf( fp, "Home %d\n",   ch->pcdata->home	);
    for (i = 0; i < 9; i++)
	fprintf( fp, "Colour%d %d\n", i, ch->pcdata->colours[i]);

    for (i = 0; i < 4; i++)
	fprintf( fp, "Faeriepath%d %d\n", i, ch->pcdata->faeriepath[i]);

    for (i = 1; i < MAX_TRADES; i++)
	fprintf( fp, "Trade %d %d\n", i, ch->trades[i]);
    fprintf(fp, "TradeRes  %d\n", ch->trade_researching);
    fprintf(fp, "TradeTime %d\n", ch->trade_time);
    for (i = 1; i < MAX_DISCIPLINES; i++)
	fprintf( fp,"Power %d %d\n", i, ch->power[i]);
    fprintf( fp, "AdvPower1 %d\n", ch->advpower[1]      );
    fprintf( fp, "AdvPower2 %d\n", ch->advpower[2]      );
    fprintf( fp, "AdvPower3 %d\n", ch->advpower[3]      );
    fprintf( fp, "AdvPower4 %d\n", ch->advpower[4]      );
    fprintf( fp, "AdvPower5 %d\n", ch->advpower[5]      );
    fprintf( fp, "AdvPower6 %d\n", ch->advpower[6]      );
    fprintf( fp, "AdvPower7 %d\n", ch->advpower[7]      );
    fprintf( fp, "AdvPower8 %d\n", ch->advpower[8]      );
    fprintf( fp, "AdvPower9 %d\n", ch->advpower[9]      );
    fprintf( fp, "AdvPower10 %d\n", ch->advpower[10]      );
    fprintf( fp, "AdvPower11 %d\n", ch->advpower[11]      );
    fprintf( fp, "AdvPower12 %d\n", ch->advpower[12]      );
    fprintf( fp, "AdvPower13 %d\n", ch->advpower[13]      );
    fprintf( fp, "Circle_Fire    %d\n", ch->fire        );
    fprintf( fp, "Circle_Water   %d\n", ch->water       );
    fprintf( fp, "Circle_Air     %d\n", ch->air         );
    fprintf( fp, "Circle_Earth   %d\n", ch->earth       );
    fprintf( fp, "Circle_Entropy %d\n", ch->entropy     );
    fprintf( fp, "Circle_Life    %d\n", ch->lifemagic   );
    fprintf( fp, "Primary_Circle %d\n",ch->primary      );
    fprintf( fp, "Current_Exp    %ld\n",ch->currentexp  );
    fprintf( fp, "Flag2 %ld\n", ch->flag2               );
    fprintf( fp, "Flag3 %ld\n", ch->flag3               );
    fprintf( fp, "Flag4 %ld\n", ch->flag4               );
    fprintf( fp, "Flag5 %ld\n", ch->flag5               );
    fprintf( fp, "Flag6 %ld\n", ch->flag6               );
    fprintf( fp, "MobSpec %ld\n", ch->mob_spec               );
    fprintf( fp, "ResFlags %ld\n", ch->res_flags               );
    fprintf( fp, "ImmFlags %ld\n", ch->imm_flags               );
    fprintf( fp, "VulFlags %ld\n", ch->vuln_flags               );
    fprintf( fp, "PKilled  %d\n",ch->pkilled            );
    fprintf( fp, "PKills   %d\n",ch->pkills             );
    fprintf( fp, "Damcap   %d\n",ch->pcdata->damcap     );
    fprintf( fp, "Upkeep   %d\n",ch->upkeep             );
    fprintf( fp, "Stance   %d\n",ch->stance             );
    fprintf( fp, "MemorizeTable\n");
    for (i = 0; i < 11; i++)
	fprintf(fp, "%d ", ch->pcdata->max_memorize[i]);
    for (i = 1; i < 11; i++)
    {
        fprintf(fp, "\n");
	for (j = 0; j < 11; j++)
	    fprintf(fp, "%d %d ", ch->pcdata->memorize[i][j],
				  ch->pcdata->last_memorized[i][j]);
    }
    fprintf(fp, "\n");

    crash("frwite char place 3");

    if (ch->pcdata->quest_item_1 != NULL)
	fprintf( fp, "QuestItem1  %s~\n",ch->pcdata->quest_item_1);
    if (ch->pcdata->quest_item_2 != NULL)
	fprintf( fp, "QuestItem2  %s~\n",ch->pcdata->quest_item_2);
    if (ch->pcdata->quest_item_3 != NULL)
	fprintf( fp, "QuestItem3  %s~\n",ch->pcdata->quest_item_3);
    if (ch->pcdata->quest_mob_1 != NULL)
	fprintf( fp, "QuestMob1  %s~\n",ch->pcdata->quest_mob_1);
    if (ch->pcdata->quest_mob_2 != NULL)
	fprintf( fp, "QuestMob2  %s~\n",ch->pcdata->quest_mob_2);
    if (ch->pcdata->quest_mob_3 != NULL)
	fprintf( fp, "QuestMob3  %s~\n",ch->pcdata->quest_mob_3);
    if (ch->pcdata->quest_room_1 != NULL)
	fprintf( fp, "QuestRoom1  %s~\n",ch->pcdata->quest_room_1);
    if (ch->pcdata->quest_room_2 != NULL)
	fprintf( fp, "QuestRoom2  %s~\n",ch->pcdata->quest_room_2);
    if (ch->pcdata->quest_room_3 != NULL)
	fprintf( fp, "QuestRoom3  %s~\n",ch->pcdata->quest_room_3);

    fprintf( fp, "QuestStatus  %ld\n",ch->pcdata->quest_status);
    fprintf( fp, "QuestsCompleted  %d\n",ch->pcdata->quests_completed);
    fprintf( fp, "ExtraAge  %d\n",ch->pcdata->extra_age);

    if (ch->pcdata->evileye != NULL)
	fprintf( fp, "Evileye  %s~\n",ch->pcdata->evileye);

    if (ch->married_to != NULL)
	fprintf( fp, "Marry    %d %s~\n",ch->marriage_status,
		ch->married_to );
    if (ch->pcdata->homepage != NULL)
	fprintf( fp, "HomePage %s~\n", ch->pcdata->homepage);
    if (ch->pcdata->email != NULL)
	fprintf( fp, "Email %s~\n", ch->pcdata->email);
    if (ch->pcdata->real_name != NULL)
	fprintf( fp, "RealName %s~\n", ch->pcdata->real_name);
    if (ch->pcdata->plan != NULL)
	fprintf( fp, "Plan %s~\n", ch->pcdata->plan);
    fprintf( fp,"StanceSkill0  %d\n",ch->stance_skill[0] );
    fprintf( fp,"StanceSkill1  %d\n",ch->stance_skill[1] );
    fprintf( fp,"StanceSkill2  %d\n",ch->stance_skill[2] );
    fprintf( fp,"StanceSkill3  %d\n",ch->stance_skill[3] );
    fprintf( fp,"StanceSkill4  %d\n",ch->stance_skill[4] );
    fprintf( fp,"StanceSkill5  %d\n",ch->stance_skill[5] );
    fprintf( fp,"StanceSkill6  %d\n",ch->stance_skill[6] );
    fprintf( fp,"StanceSkill7  %d\n",ch->stance_skill[7] );
    fprintf( fp,"StanceSkill8  %d\n",ch->stance_skill[8] );
    fprintf( fp,"StanceSkill9  %d\n",ch->stance_skill[9] );
    fprintf( fp,"StanceSkill10 %d\n",ch->stance_skill[10] );
    fprintf( fp,"StanceSkill11 %d\n",ch->stance_skill[11] );
    fprintf( fp,"StanceSkill12 %d\n",ch->stance_skill[12] );

    crash("frwite char place 4");

    for (i = 0; i < MAX_TIMER; i++)
	fprintf( fp,"Tick_Timer %d %d\n", i, ch->tick_timer[i]);

    for (i = 0; i < MAX_SKILL_TREE; i++)
	fprintf( fp,"TreeSkill %d %d\n", i, ch->pcdata->treelearn[i]);

    if (ch->secondaryrace == RACE_VAMPIRE)
    {
	for (i = 0; i < 13; i++)
	{
	    if (ch->pcdata->lineage[i] != NULL)
		fprintf( fp, "Lineage %s~\n", ch->pcdata->lineage[i] );
	}
    }
   /* fprintf( fp, "Conditioned_By %s",ch->pcdata->conditioned_by);*/
    buf[0] = '\0';
    for (i = 0; i < NUM_VALS_MOBS; i++)
    {
	sprintf(buf, "RaceVal %d %%d\n",i);
	fprintf(fp, buf, ch->value[i]);
    }
    fprintf( fp,"Style %d %d %d %d %d %d %d %d\n",
	ch->style_skill[0],
	ch->style_skill[1],
	ch->style_skill[2],
	ch->style_skill[3],
	ch->style_skill[4],
	ch->style_skill[5],
	ch->style_skill[6],
	ch->style_skill[7]);
    fprintf( fp,"PsiStuff %d %d %d %d %d %d %d %d\n",
	ch->psp,
	ch->max_psp,
	ch->psi_level,
	ch->psi_potential,
	ch->pcdata->psi_disc_points,
	ch->pcdata->psi_sci_points,
	ch->pcdata->psi_dev_points,
	ch->pcdata->psi_def_points);

    for (sn = 1; sn < 8; sn++)
	fprintf(fp, "PsiBits %d %d\n", sn, ch->pcdata->psi_disc[sn]);

    crash("frwite char place 5");

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
	if (ch->psi_learnt[sn])
	    fprintf( fp, "PsiSkill %d\n",sn);
    }
    fprintf( fp, "ElderPowers %d %d %d %d %d %d %d %d\n",
	ch->elder[1],
	ch->elder[2],
	ch->elder[3],
	ch->elder[4],
	ch->elder[5],
	ch->elder[6],
	ch->elder[7],
	ch->elder[8]);
    if (ch->trust != 0)
	fprintf( fp, "Tru  %d\n",       ch->trust       );
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)   );
    fprintf( fp, "Not  %ld %ld %ld %ld %ld\n",          
	ch->pcdata->last_note,ch->pcdata->last_idea,ch->pcdata->last_penalty,
	ch->pcdata->last_news,ch->pcdata->last_changes  );
    fprintf( fp, "Scro %d\n",   ch->lines               );
    fprintf( fp, "Realm %s~\n",  ch->realm->name	);
    fprintf( fp, "Room %d\n",
	(  ch->in_room == get_room_index( prime_realm, ROOM_VNUM_LIMBO )
	&& ch->was_in_room != NULL )
	    ? ch->was_in_room->vnum
	    : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

    fprintf( fp, "HV %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->move, ch->max_move );
    if (ch->gold > 0)
      fprintf( fp, "Gold %ld\n",        ch->gold                );
    else
      fprintf( fp, "Gold %d\n", 0                       ); 
    if (ch->silver > 0)
	fprintf( fp, "Silv %ld\n",ch->silver            );
    else
	fprintf( fp, "Silv %d\n",0                      );


    fprintf( fp, "Education  %d\n",   ch->pcdata->education                 );
    fprintf( fp, "Emeralds  %ld\n",   ch->emeralds                 );
    fprintf( fp, "Copper  %ld\n",   ch->copper                 );
    fprintf( fp, "Diamonds  %ld\n",   ch->diamonds                 );
    fprintf( fp, "Sapphires  %ld\n",   ch->sapphires                 );
    fprintf( fp, "Exp  %d\n",   ch->exp                 );
    fprintf( fp, "DragType  %d\n",   ch->dragtype                 );
    fprintf( fp, "DiscRese  %d\n",   ch->pcdata->disc_research              );
    fprintf( fp, "DiscPoin  %d\n",   ch->pcdata->disc_points          );

    fprintf( fp, "Aura  %d\n",   ch->aura                 );
    if (ch->act != 0)
	fprintf( fp, "Act  %s\n",   print_flags(ch->act));
    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));
    fprintf( fp, "Comm %s\n",       print_flags(ch->comm));
    if (ch->wiznet)
	fprintf( fp, "Wizn %s\n",   print_flags(ch->wiznet));
    if (ch->invis_level)
	fprintf( fp, "Invi %d\n",       ch->invis_level );
    if (ch->incog_level)
	fprintf(fp,"Inco %d\n",ch->incog_level);
    fprintf( fp, "Pos  %d\n",   
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",      ch->saving_throw);
    fprintf( fp, "Alig  %d\n",  ch->alignment           );
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",      ch->hitroll     );
    if (ch->xhitroll != 0)
	fprintf( fp, "XHitroll   %d\n",      ch->xhitroll     );
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",      ch->damroll     );
    if (ch->xdamroll != 0)
	fprintf( fp, "XDamroll   %d\n",      ch->xdamroll     );
    fprintf( fp, "ACs %d %d %d %d\n",   
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",      ch->wimpy       );
    fprintf( fp, "Attr %d %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON],
	ch->perm_stat[STAT_CHA] );

    crash("frwite char place 6");

    fprintf (fp, "AMod %d %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON],
	ch->mod_stat[STAT_CHA] );

    fprintf (fp, "Current_Form %d\n", ch->cur_form);

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",       ch->pIndexData->vnum    );
    }
    else
    {
	fprintf( fp, "Pass %s~\n",      ch->pcdata->pwd         );
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf( fp, "Bin  %s~\n",  ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
		fprintf( fp, "Bout %s~\n",      ch->pcdata->bamfout);
	fprintf( fp, "Titl %s~\n",      ch->pcdata->title       );
	if (ch->pcdata->title_prefix != NULL)
	    fprintf( fp, "TitlePre %s~\n",      ch->pcdata->title_prefix   );
	fprintf( fp, "CERT %s~\n",      ch->pcdata->custom_enter_room);
	fprintf( fp, "Pnts %d\n",       ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",       ch->pcdata->true_sex    );
	fprintf( fp, "LLev %d\n",       ch->pcdata->last_level  );
	fprintf( fp, "HVP %d %d\n", ch->pcdata->perm_hit, 
						   ch->pcdata->perm_move);
	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );

	/* write alias */
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (ch->pcdata->alias[pos] == NULL
	    ||  ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	    if ( skill_table[sn].name != NULL
		&& ch->pcdata->innate_max[sn] > 0)
	    {
		fprintf( fp, "Innate %d '%s' %d %d %d\n",
		    ch->pcdata->innate[sn],
		    skill_table[sn].name,
		    ch->pcdata->innate_max[sn],
		    ch->pcdata->innate_regain[sn],
		    ch->pcdata->innate_timer[sn]);
	    }
	}

    }

    crash("frwite char place 7");

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ((paf->type < 0 || paf->type>= MAX_SKILL) && paf->name == NULL)
	    continue;
	
	crash("writing an affect to a save file");

	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %s~ %s~\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector,
	    (paf->name != NULL) ? paf->name : "NOTHING",
	    (paf->end_message != NULL) ? paf->end_message : "NOTHING"
	);
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;

    if (pet		== NULL) return;
    if (pet->pIndexData == NULL) return;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
	fprintf( fp, "Clan %s~\n",clan_table[pet->clan].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HV  %d %d %d %d\n",
	pet->hit, pet->max_hit,  pet->move, pet->max_move);
    if (pet->gold > 0)
	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->power[DISC_WARL_CONT] > 0)
	fprintf(fp,"MagRes %d\n",pet->power[DISC_WARL_CONT]);
    if (pet->exp > 0)
	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->flag2 != 0)
	fprintf(fp, "Flag2 %ld\n", pet->flag2);
    if (pet->flag3 != 0)
	fprintf(fp, "Flag3 %ld\n", pet->flag3);
    if (pet->flag4 != 0)
	fprintf(fp, "Flag4 %ld\n", pet->flag4);
    if (pet->flag5 != 0)
	fprintf(fp, "Flag5 %ld\n", pet->flag5);
    if (pet->flag6 != 0)
	fprintf(fp, "Flag6 %ld\n", pet->flag6);
    if (pet->comm != 0)
	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d %d\n",
	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
	pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHA]);
    fprintf(fp, "AMod %d %d %d %d %d %d\n",
	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
	pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHA]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
	if ((paf->type < 0 || paf->type>= MAX_SKILL) && paf->name == NULL)
	    continue;
	    
	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %s~ %s~\n",
	    skill_table[paf->type].name,
	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
	    paf->bitvector,
	    (paf->name != NULL) ? paf->name : "NOTHING",
	    (paf->end_message != NULL) ? paf->end_message : "NOTHING");
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",   iNest                );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
	fprintf( fp, "Name %s~\n",      obj->name                    );
    if ( obj->short_descr != obj->pIndexData->short_descr)
	fprintf( fp, "ShD  %s~\n",      obj->short_descr             );
    if ( obj->description != obj->pIndexData->description)
	fprintf( fp, "Desc %s~\n",      obj->description             );
    fprintf( fp, "ExtF %d\n",        obj->extra_flags              );
    fprintf( fp, "ExtF2 %d\n",       obj->extra_flags2             );
    fprintf( fp, "ExtF3 %d\n",       obj->extra_flags3             );
    fprintf( fp, "WeaF %d\n",       obj->wear_flags              );
    fprintf( fp, "Ityp %d\n",       obj->item_type               );
    fprintf( fp, "Wt   %d\n",       obj->weight                  );
    fprintf( fp, "PermSpell %d\n",       obj->perm_spell               );
    if ( obj->condition != 100)
        fprintf( fp, "Condition %d\n",       obj->condition               );
    if ( obj->size != SIZE_MEDIUM )
	fprintf( fp, "Size %d\n",       obj->size               );
    if ( obj->last_owner != NULL)
	fprintf( fp, "LastOwner %s~\n",  obj->last_owner               );
    if ( obj->voodoo_name != NULL)
	fprintf( fp, "VoodooName %s~\n",  obj->voodoo_name               );
    if ( obj->material != NULL)
	fprintf( fp, "Material %s~\n",  obj->material    );

    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
	fprintf( fp, "Lev  %d\n",       obj->level                   );
    if (obj->timer != 0)
	fprintf( fp, "Time %d\n",       obj->timer           );
    fprintf( fp, "Cost %d\n",   obj->cost                    );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]            );

    if (obj->value[5] != obj->pIndexData->value[5]
    ||  obj->value[6] != obj->pIndexData->value[6]
    ||  obj->value[7] != obj->pIndexData->value[7]
    ||  obj->value[8] != obj->pIndexData->value[8]
    ||  obj->value[9] != obj->pIndexData->value[9]) 
	fprintf( fp, "Val2 %d %d %d %d %d\n",
	    obj->value[5], obj->value[6], obj->value[7], obj->value[8],
	    obj->value[9]            );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ((paf->type < 0 || paf->type>= MAX_SKILL) && paf->name == NULL)
	    continue;
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d %s~ %s~\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector,
	    (paf->name != NULL) ? paf->name : "NOTHING",
	    (paf->end_message != NULL) ? paf->end_message : "NOTHING"
	    );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */

bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    CHAR_DATA *ch;

    crash("load char obj 1");

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character                        = ch;
    ch->desc                            = d;
    return do_load_char_obj( ch, name );
}

bool do_load_char_obj( CHAR_DATA *ch, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    FILE *fp;
    bool found;
    int stat,sn,i,p;
    char *tmp;

    crash("do load char obj 2");

    ch->name                            = str_dup( name );
    ch->id                              = get_pc_id();
    ch->race                            = race_lookup("human");
    ch->act                             = PLR_NOSUMMON;
    ch->comm                            = COMM_COMBINE | COMM_PROMPT;
    ch->prompt                          = str_dup("<%hhp %vmv %xexp> ");
    ch->fightprompt                     = str_dup("<%o %hhp %ppsp %vmv [%d]> ");
    ch->tempprompt			= NULL;
    ch->build_realm			= 1;
    ch->registration_key		= 0;
    ch->des_dist			= 0;
    ch->fight_style			= FS_AGGRESSIVE + FS_CONCENTRATE;
    ch->registration_logins		= 0;
    ch->email_set			= FALSE;
    ch->registerred			= FALSE;
    ch->pcdata->confirm_delete          = FALSE;
    ch->pcdata->curr_vnum_medit		= -1;
    ch->pcdata->curr_vnum_oedit		= -1;
    ch->pcdata->education		= 0;
    ch->pcdata->quests_completed	= 0;
    ch->pcdata->quest_status		= 0;
    ch->pcdata->evileye			= NULL;
    ch->pcdata->pwd                     = str_dup( "" );
    ch->pcdata->bamfin                  = str_dup( "" );
    ch->pcdata->bamfout                 = str_dup( "" );
    ch->pcdata->title                   = str_dup( "" );
    ch->pcdata->racial_enemy		= str_dup( "" );
    ch->pcdata->title_prefix            = NULL;
    ch->pcdata->custom_enter_room	= str_dup( "" );

    crash("do load char obj 3");

    ch->pcdata->switchname              = str_dup( ch->name );
    ch->mob_spec			= 0;
    ch->pcdata->extra_age		= 0;
    ch->pcdata->quests_completed	= 0;
    ch->pcdata->quest_status		= 0;
    ch->pcdata->quest_item_1		= NULL;
    ch->pcdata->quest_item_2		= NULL;
    ch->pcdata->quest_item_3		= NULL;
    ch->pcdata->quest_mob_1		= NULL;
    ch->pcdata->quest_mob_2		= NULL;
    ch->pcdata->quest_mob_3		= NULL;
    ch->pcdata->quest_room_1		= NULL;
    ch->pcdata->quest_room_2		= NULL;
    ch->pcdata->quest_room_3		= NULL;
    ch->auto_look_line			= -1;
    ch->faith 				= 0;
    ch->true_faith 			= 0;
    ch->riding				= NULL;
    ch->riden_by			= NULL;
    ch->sw_obj				= NULL;
    ch->flag2 				= 0;
    ch->flag3 				= 0;
    ch->flag4 				= 0;
    ch->flag5 				= 0;
    ch->flag6 				= 0;
    ch->realm				= prime_realm;
    ch->pcdata->sanity			= 0;
    ch->aura				= 0;

    /* Eladrin promote - just to shut them up! */
	    for (p = 0 ; p < 4 ; p++)
	ch->pcdata->faeriepath[p]       = 0;

    for (p = 0; p < 9; p++)
	ch->pcdata->colours[p] = 3;

    for (p = 0 ; p < MAX_SKILL; p++)
    {
	ch->pcdata->innate[p] 		= 0;
	ch->pcdata->innate_max[p] 	= 0;
	ch->pcdata->innate_regain[p] 	= 0;
 	ch->pcdata->innate_timer[p]	= 0;
    }
    crash("do load char obj 4");


      for (p = 0 ; p < 11 ; p++)
            {
                ch->pcdata->max_memorize[p] = 0;
                for (i = 0 ; i < 11 ; i++)
                {
                    ch->pcdata->memorize[p][i] = -1;
                    ch->pcdata->last_memorized[p][i] = -1;
                }
            }
    for (p = 0 ; p < MAX_TRADES; p++)
    {
	ch->trades[p] = -1;
    }
    ch->trade_researching = -1;
    ch->trade_time = -1;

    for (stat =0; stat < MAX_STATS; stat++)
	{
		ch->perm_stat[stat]             = 13;
		ch->pcdata->condition[COND_THIRST]  = 48; 
		ch->pcdata->condition[COND_FULL]    = 48;
		ch->pcdata->condition[COND_HUNGER]  = 48;
	}
	
	for (i = 0; i < 13; i++)
	{
		ch->pcdata->lineage[i] = NULL;
	}
	
    crash("do load char obj 5");
    ch->pcdata->conditioned_by = NULL;

    ch->style_skill[0] = 0;
    ch->style_skill[1] = 0;
    ch->style_skill[2] = 0;
    ch->style_skill[3] = 0;
    ch->style_skill[4] = 0;
    ch->style_skill[5] = 0;
    ch->style_skill[6] = 0;
    ch->style_skill[7] = 0;
    ch->curlang = -1;
    ch->pcdata->damcap = 200;

    for (sn = 0; sn < NUM_PSI_SLOTS; sn++)
    {
		ch->psi_learnt[sn] = 0;
		ch->psi_upkeep[sn].dur = -1;
    }

    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    crash("do load char obj 6");

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
        crash("do load char obj 7");
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    crash("do load char obj 8");
    fpReserve = fopen( NULL_FILE, "r" );

    if (ch->pet != NULL)
	char_to_room( ch->pet, get_room_index(prime_realm, 47), 0);

    /* initialize race */
    if (found)
    {
	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->dam_type = 17; /*punch */

	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->imm_flags   = ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags   = ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags  = ch->vuln_flags | race_table[ch->race].vuln;
	ch->form        = race_table[ch->race].form;
	ch->parts       = race_table[ch->race].parts;
    }

    if (ch->curlang == -1) ch->curlang = 3;
    crash("do load char obj 9");

    tmp = str_dup(ch->name);
    set_pc_name(ch, ch->pcdata->switchname);
    set_switchname(ch, tmp);

    /* check they have all teh spells they should have */
    {
	int i; 

    	for (i = 1; i < ch->lifemagic; i++)
	    gn_add(ch, i + 59);
    	for (i = 1; i < ch->fire; i++)
	    gn_add(ch, i + 9);
    	for (i = 1; i < ch->earth; i++)
	    gn_add(ch, i + 39);
    	for (i = 1; i < ch->water; i++)
	    gn_add(ch, i + 19);
    	for (i = 1; i < ch->air; i++)
	    gn_add(ch, i + 29);
    	for (i = 1; i < ch->entropy; i++)
	    gn_add(ch, i + 49);

    }

    if (ch->version < 10)
    {
	ch->version = 10;

	if (ch->secondaryrace != 0)
	{
	    int i;

	    for (i = 0; i < MAX_DISCIPLINES; i++)
		ch->power[i] = -2;

	    ch->aura = 0;
	    ch->tertiaryrace = 0;
	    ch->clan = 0;
	    ch->level = 20;
	    ch->exp = 20000;
	    ch->pcdata->disc_research = -1;
	    ch->pcdata->disc_points = 0;
	    if (ch->pcdata->evileye != NULL) ch->pcdata->evileye[0] = 0;
	}
    }

    /*check_balance(ch);*/

    /* See if they are switched */
    if (IS_FLAG6(ch, AFF6_OBJ_SWITCHED))
    {
	set_form(ch, get_normal_form(ch));
	set_pc_name(ch, GET_PROPER_NAME(ch));
	REMOVE_BIT(ch->flag6, AFF6_OBJ_SWITCHED);
    }

    crash("do load char obj 10");
    check_player_table(ch);

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value ) if ( !str_cmp( word, literal ) ) { field  = value; fMatch = TRUE; break; }

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value ) if ( !str_cmp( word, literal ) ) { free_string(field); field  = value; fMatch = TRUE; break; }

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int count = 0;
    int lastlogoff = current_time;
    int percent;

    sprintf(buf,"Loading %s.",ch->name);
    log_string(buf);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",         ch->act,                fread_flag( fp ) );
	    KEY( "AffectedBy",  ch->affected_by,        fread_flag( fp ) );
	    KEY( "AfBy",        ch->affected_by,        fread_flag( fp ) );
	    KEY( "Alignment",   ch->alignment,          fread_number( fp ) );
	    KEY( "Aura",   	ch->aura,          fread_number( fp ) );
	    KEY( "Altitude", 	ch->altitude,		fread_number( fp ) );
	    KEY( "Alig",        ch->alignment,          fread_number( fp ) );
	    KEY( "AutoLookLine",ch->auto_look_line,	fread_number( fp ) );

	    if (!str_cmp( word, "Alia"))
	    {
		if (count >= MAX_ALIAS)
		{
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}

		ch->pcdata->alias[count]        = str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count]    = str_dup(fread_word(fp));
		count++;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp( word, "Alias"))
	    {
		if (count >= MAX_ALIAS)
		{
		    fread_to_eol(fp);
		    fMatch = TRUE;
		    break;
		}
 
		ch->pcdata->alias[count]        = str_dup(fread_word(fp));
		ch->pcdata->alias_sub[count]    = fread_string(fp);
		count++;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level      = fread_number( fp );
		paf->duration   = fread_number( fp );
		paf->modifier   = fread_number( fp );
		paf->location   = fread_number( fp );
		paf->bitvector  = fread_number( fp );
	 	paf->name	= fread_string( fp );
		if (!str_cmp(paf->name,"NOTHING"))
			free_string(paf->name);
	 	paf->end_message	= fread_string( fp );
		if (!str_cmp(paf->end_message,"NOTHING"))
			free_string(paf->end_message);
		paf->next       = ch->affected;
		ch->affected    = paf;
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "Affc"))
	    {
		AFFECT_DATA *paf;
		int sn;
 
		paf = new_affect();
 
		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;
 
		paf->where  = fread_number(fp);
		paf->level      = fread_number( fp );
		paf->duration   = fread_number( fp );
		paf->modifier   = fread_number( fp );
		paf->location   = fread_number( fp );
		paf->bitvector  = fread_number( fp );
	 	paf->name	= fread_string( fp );
		if (!str_cmp(paf->name,"NOTHING"))
		{
		    free_string(paf->name);
		    paf->name = NULL;
		}	
	 	paf->end_message	= fread_string( fp );
		if (!str_cmp(paf->end_message,"NOTHING"))
		{
		    free_string(paf->end_message);
		    paf->end_message = NULL;
		}
		paf->next       = ch->affected;
		ch->affected    = paf;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    KEY( "AdvPower1",    ch->advpower[1],        fread_number( fp ) );
	    KEY( "AdvPower2",    ch->advpower[2],        fread_number( fp ) );
	    KEY( "AdvPower3",    ch->advpower[3],        fread_number( fp ) );
	    KEY( "AdvPower4",    ch->advpower[4],        fread_number( fp ) );
	    KEY( "AdvPower5",    ch->advpower[5],        fread_number( fp ) );
	    KEY( "AdvPower6",    ch->advpower[6],        fread_number( fp ) );
	    KEY( "AdvPower7",    ch->advpower[7],        fread_number( fp ) );
	    KEY( "AdvPower8",    ch->advpower[8],        fread_number( fp ) );
	    KEY( "AdvPower9",    ch->advpower[9],        fread_number( fp ) );
	    KEY( "AdvPower10",    ch->advpower[10],        fread_number( fp ) );
	    KEY( "AdvPower11",    ch->advpower[11],        fread_number( fp ) );
	    KEY( "AdvPower12",    ch->advpower[12],        fread_number( fp ) );
	    KEY( "AdvPower13",    ch->advpower[13],        fread_number( fp ) );
	    break;
 
	case 'B':
	    KEY( "Bamfin",      ch->pcdata->bamfin,     fread_string( fp ) );
	    KEY( "Bamfout",     ch->pcdata->bamfout,    fread_string( fp ) );
	    KEY( "Bin",         ch->pcdata->bamfin,     fread_string( fp ) );
	    KEY( "Bout",        ch->pcdata->bamfout,    fread_string( fp ) );
	    KEY( "BodyForm",    ch->body_form,          fread_number( fp ) );
	    KEY( "BestMobKilled",    ch->best_mob_killed,fread_number( fp ) );
	    KEY( "BuildRealm",	ch->build_realm,	fread_number( fp ) );
	    KEY( "Breaths",	ch->breaths,		fread_number( fp ) );
	    KEY( "Blood",	ch->blood,		fread_number( fp ) );
	    KEY( "Beast",	ch->beast,		fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "Copper", 	ch->copper,		fread_number( fp ) );
	    KEY( "CurLang",     ch->curlang,            fread_number( fp ) );
	    KEY( "Colour0",     ch->pcdata->colours[0], fread_number( fp ) );
	    KEY( "Colour1",     ch->pcdata->colours[1], fread_number( fp ) );
	    KEY( "Colour2",     ch->pcdata->colours[2], fread_number( fp ) );
	    KEY( "Colour3",     ch->pcdata->colours[3], fread_number( fp ) );
	    KEY( "Colour4",     ch->pcdata->colours[4], fread_number( fp ) );
	    KEY( "Colour5",     ch->pcdata->colours[5], fread_number( fp ) );
	    KEY( "Colour6",     ch->pcdata->colours[6], fread_number( fp ) );
	    KEY( "Colour7",     ch->pcdata->colours[7], fread_number( fp ) );
	    KEY( "Colour8",     ch->pcdata->colours[8], fread_number( fp ) );
	    KEY( "Clan",        ch->clan,       clan_lookup(fread_string(fp)));
	    KEY( "Circle_Fire", ch->fire,               fread_number( fp ) );
	    KEY( "Circle_Water", ch->water,             fread_number( fp ) );
	    KEY( "Circle_Air", ch->air,                 fread_number( fp ) );
	    KEY( "Circle_Earth", ch->earth,             fread_number( fp ) );
	    KEY( "Circle_Entropy", ch->entropy,         fread_number( fp ) );
	    KEY( "Circle_Life", ch->lifemagic,         fread_number( fp ) );
	    KEY( "Current_Exp", ch->currentexp,         fread_number( fp ) );
	    KEY( "Current_Form", ch->cur_form,          fread_number( fp ) );
	    KEYS("Conditioned_By", ch->pcdata->conditioned_by, fread_string(fp));
	    KEYS("CERT", ch->pcdata->custom_enter_room, fread_string(fp) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word,"Cnd"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY("Comm",         ch->comm,               fread_flag( fp ) ); 
	  
	    break;

	case 'D':
	    KEY( "DiscRese", 	ch->pcdata->disc_research, 	fread_number( fp ) );
	    KEY( "DiscPoin", 	ch->pcdata->disc_points,		fread_number( fp ) );
	    KEY( "DragType", 	ch->dragtype,		fread_number( fp ) );
	    KEY( "DesiredDist", ch->des_dist,		fread_number( fp ) );
	    KEY( "Diamonds", 	ch->diamonds,		fread_number( fp ) );
	    KEY( "Damroll",     ch->damroll,            fread_number( fp ) );
	    KEY( "Dam",         ch->damroll,            fread_number( fp ) );
	    KEY( "Description", ch->description,        fread_string( fp ) );
	    KEY( "Desc",        ch->description,        fread_string( fp ) );
	    KEY( "Damcap",      ch->pcdata->damcap,     fread_number( fp ) );
	    break;

	case 'E':
	    KEY( "Education", 	ch->pcdata->education,	fread_number( fp ) );
	    KEY( "Emeralds", 	ch->emeralds,	fread_number( fp ) );
	    KEY( "ExtraAge", ch->pcdata->extra_age, 
		fread_number(fp));
	    KEY( "Email", 	ch->pcdata->email,	fread_string( fp ) );
	    KEY( "EmailSet", 	ch->email_set,		fread_number( fp ) );
	    KEY( "Evileye",     ch->pcdata->evileye, fread_string( fp ) );
	    if ( !str_cmp( word, "End" ) )
	    {
		/* adjust hp mana move up  -- here for speed's sake */
		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
		{
		    ch->hit     += (ch->max_hit - ch->hit) * percent / 100;
		    ch->move    += (ch->max_move - ch->move)* percent / 100;
		}
		return;
	    }
	    KEY( "Exp",         ch->exp,                fread_number( fp ) );
	    if ( !str_cmp( word, "ElderPowers"))
	    {
		int stat;
		for (stat = 1; stat < 9; stat++)
			ch->elder[stat] = fread_number(fp);
		fMatch = TRUE;
	    }
	    
	    
	    break;

	case 'F':
	   KEYS( "FightPrompt",      ch->fightprompt,  fread_string( fp ) );
	   KEY( "Fth", ch->faith        , fread_number( fp ) );
	   KEY( "FthTrue", ch->true_faith        , fread_number( fp ) );
	   KEY( "Form", ch->secondaryrace, fread_number( fp ) );
	   KEY( "FightStyle", ch->fight_style,     fread_number( fp ) );
	   KEY( "Faeriepath0",     ch->pcdata->faeriepath[0], fread_number( fp ) );
	   KEY( "Faeriepath1",     ch->pcdata->faeriepath[1], fread_number( fp ) );
	   KEY( "Faeriepath2",     ch->pcdata->faeriepath[2], fread_number( fp ) );
	   KEY( "Faeriepath3",     ch->pcdata->faeriepath[3], fread_number( fp ) );
	   KEY( "Flag2", ch->flag2,     fread_number( fp ) );
	   KEY( "Flag3", ch->flag3,     fread_number( fp ) );
	   KEY( "Flag4", ch->flag4,     fread_number( fp ) );
	   KEY( "Flag5", ch->flag5,     fread_number( fp ) );
	   KEY( "Flag6", ch->flag6,     fread_number( fp ) );
	   break;

	case 'G':
	    KEY( "Grant",       ch->grant_cmds,         fread_number( fp ) );
	    KEY( "Gold",        ch->gold,               fread_number( fp ) );
	    KEY( "Gen",         ch->generation,         fread_number( fp ) );
	    break;

	case 'H':
	    KEY( "Hitroll",     ch->hitroll,            fread_number( fp ) );
	    KEY( "Hit",         ch->hitroll,            fread_number( fp ) );
	    KEY( "HomePage", 	ch->pcdata->homepage,	fread_string( fp ) );
	    KEY( "Home", 	ch->pcdata->home,	fread_number( fp ) );
	    if ( !str_cmp( word, "HpMove" ) || !str_cmp(word,"HV"))
	    {
		ch->hit         = fread_number( fp );
		ch->max_hit     = fread_number( fp );
		ch->move        = fread_number( fp );
		ch->max_move    = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "HpMovePerm" ) || !str_cmp(word,"HVP"))
	    {
		ch->pcdata->perm_hit    = fread_number( fp );
		ch->pcdata->perm_move   = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
      
	    break;

	case 'I':
	    KEY( "Id",          ch->id,                 fread_number( fp ) );
	    KEY( "InvisLevel",  ch->invis_level,        fread_number( fp ) );
	    KEY( "Inco",        ch->incog_level,        fread_number( fp ) );
	    KEY( "Invi",        ch->invis_level,        fread_number( fp ) );
	    KEY( "ImmPower",    ch->immpower,           fread_number( fp ) );
	    KEY( "ImmFlags",    ch->imm_flags,          fread_number( fp ) );
	    if ( !str_cmp( word, "Innate" ))
	    {
		int sn;
		int value, v2, v3, v4;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		v2 = fread_number( fp );
		v3 = fread_number( fp );
		v4 = fread_number( fp );
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		{
		    ch->pcdata->innate[sn] = value;
		    ch->pcdata->innate_max[sn] = v2;
		    ch->pcdata->innate_regain[sn] = v3;
		    ch->pcdata->innate_timer[sn] = v4;
		}
		fMatch = TRUE;
	    }

	    break;

	case 'L':
	    KEY( "LastLevel",   ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",        ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",       ch->level,              fread_number( fp ) );
	    KEY( "Lev",         ch->level,              fread_number( fp ) );
	    KEY( "Levl",        ch->level,              fread_number( fp ) );
	    KEY( "LogO",        lastlogoff,             fread_number( fp ) );
	    KEY( "LongDescr",   ch->long_descr,         fread_string( fp ) );
	    KEY( "LnD",         ch->long_descr,         fread_string( fp ) );
	    KEYS("LastLoginFrom",ch->pcdata->last_login_from, fread_string( fp ) );
	    KEYS("LastLoginTime",ch->pcdata->last_login_time, fread_string( fp ) );
	    if (!str_cmp(word, "Lineage"))
	    {
		int i=0;
		while (ch->pcdata->lineage[i]!=NULL && i < 13) i++;
		ch->pcdata->lineage[i]=str_dup(fread_string( fp ));
		fMatch = TRUE;
	    }

	    break;

	case 'M':
	    KEY( "MobSpec",        ch->mob_spec,  fread_number(fp ) );
	    KEY( "MobKills",        ch->pcdata->mob_kills,  fread_number(fp ) );
	    KEY( "MobKilled",        ch->pcdata->mob_killed,fread_number(fp ) );
	    if (!str_cmp(word, "MakeClanLeader") )
	    {
		    if (ch->version < 10)
		    {
			ch->version = 10;
	
			if (ch->secondaryrace != 0)
			{
			    int i;
		
			    for (i = 0; i < MAX_DISCIPLINES; i++)
				ch->power[i] = -2;
		
			    ch->aura = 0;
			    ch->tertiaryrace = 0;
			    ch->clan = 0;
			    ch->level = 20;
			    ch->exp = 20000;
			    ch->pcdata->disc_research = -1;
			    ch->pcdata->disc_points = 0;
			    if (ch->pcdata->evileye != NULL) ch->pcdata->evileye[0] = 0;
			}
		    }
		ch->clan = clan_lookup(fread_string(fp));
		ch->generation = 12;
		WRITE_STR(clan_data[ch->clan].leader, GET_PROPER_NAME(ch) );
		ch->power[clan_table[ch->clan].disc1] = 7;
		ch->power[clan_table[ch->clan].disc2] = 7;
		ch->power[clan_table[ch->clan].disc3] = 7;
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word, "Marry") )
	    {
		ch->marriage_status = fread_number(fp);
		ch->married_to = str_dup(fread_string(fp));
		fMatch = TRUE;
		break;
	    }
	    if (!str_cmp(word, "MemorizeTable") )
	    {
		int i, j;
		for (i = 0; i < 11; i++)
		    ch->pcdata->max_memorize[i] = fread_number( fp );
		for (i = 1; i < 11; i++)
		{
		    for (j = 0; j < 11; j++)
		    {
			ch->pcdata->memorize[i][j] = fread_number(fp);
			ch->pcdata->last_memorized[i][j] = fread_number( fp );
		    }
	 	}
		fMatch = TRUE;
		break;
	    }

	case 'N':
	    KEYS( "Name",       ch->name,               fread_string( fp ) );
	    KEY( "Note",        ch->pcdata->last_note,  fread_number( fp ) );
	    if (!str_cmp(word,"Not"))
	    {
		ch->pcdata->last_note                   = fread_number(fp);
		ch->pcdata->last_idea                   = fread_number(fp);
		ch->pcdata->last_penalty                = fread_number(fp);
		ch->pcdata->last_news                   = fread_number(fp);
		ch->pcdata->last_changes                = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'P':
	    KEY( "Password",    ch->pcdata->pwd,        fread_string( fp ) );
	    KEY( "Pass",        ch->pcdata->pwd,        fread_string( fp ) );
	    KEY( "PKilled",     ch->pkilled,            fread_number( fp ) );
	    KEY( "PKills",      ch->pkills,             fread_number( fp ) );
	    KEY( "Played",      ch->played,             fread_number( fp ) );
	    KEY( "Plyd",        ch->played,             fread_number( fp ) );
	    KEY( "Plan", 	ch->pcdata->plan,	fread_string( fp ) );
	    KEY( "Points",      ch->pcdata->points,     fread_number( fp ) );
	    KEY( "Pnts",        ch->pcdata->points,     fread_number( fp ) );
	    KEY( "Position",    ch->position,           fread_number( fp ) );
	    KEY( "Pos",         ch->position,           fread_number( fp ) );
 	    if ( !str_cmp( word, "Power" ) )
	    {
		ch->power[fread_number(fp)] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    KEYS( "Prompt",      ch->prompt,             fread_string( fp ) );
	    KEY( "PromTemp",   ch->tempprompt,         fread_string( fp ) );
	    KEY( "Prom",        ch->prompt,             fread_string( fp ) );
	    KEY( "Primary_Circle", ch->primary,         fread_number( fp ) );
	    if (!str_cmp(word, "PsiSkill"))
	    {
		ch->psi_learnt[fread_number(fp)] = 1;
		fMatch = TRUE;
	    }
	    if (!str_cmp(word, "PsiBits"))
	    {
		ch->pcdata->psi_disc[fread_number(fp)] = fread_number(fp);
		fMatch = TRUE;
	    }
	    if (!str_cmp(word, "PsiStuff"))
	    {
		ch->psp                                 = fread_number(fp);
		ch->max_psp                             = fread_number(fp);
		ch->psi_level                           = fread_number(fp);
		ch->psi_potential                       = fread_number(fp);
		ch->pcdata->psi_disc_points		= fread_number(fp);
		ch->pcdata->psi_sci_points		= fread_number(fp);
		ch->pcdata->psi_dev_points		= fread_number(fp);
		ch->pcdata->psi_def_points		= fread_number(fp);
		fMatch = TRUE;
	    }
	    if (!str_cmp(word, "Psionics"))
	    {
		ch->psp                                 = fread_number(fp);
		ch->max_psp                             = fread_number(fp);
		ch->psi_level                           = fread_number(fp);
		ch->psi_potential                       = fread_number(fp);
		ch->psp = 0;
		ch->max_psp = 0;
		ch->psi_level = 0;
		fMatch = TRUE;
	    }
	    break;
	case 'Q':
	    KEY( "QuestStatus", ch->pcdata->quest_status, 
		fread_number(fp));
	    KEY( "QuestsCompleted", ch->pcdata->quests_completed, 
		fread_number(fp));
	    KEY( "QuestItem1",ch->pcdata->quest_item_1, fread_string( fp ) );
	    KEY( "QuestItem2",ch->pcdata->quest_item_2, fread_string( fp ) );
	    KEY( "QuestItem3",ch->pcdata->quest_item_3, fread_string( fp ) );
	    KEY( "QuestMob1", ch->pcdata->quest_mob_1,  fread_string( fp ) );
	    KEY( "QuestMob2", ch->pcdata->quest_mob_2,  fread_string( fp ) );
	    KEY( "QuestMob3", ch->pcdata->quest_mob_3,  fread_string( fp ) );
	    KEY( "QuestRoom1",ch->pcdata->quest_room_1, fread_string( fp ) );
	    KEY( "QuestRoom2",ch->pcdata->quest_room_2, fread_string( fp ) );
	    KEY( "QuestRoom3",ch->pcdata->quest_room_3, fread_string( fp ) );
	    KEY( "QP",          ch->qp,                 fread_number( fp ) );
	    KEY( "QProg",       ch->quest_progress,     fread_number( fp ) );
	    break;
	case 'R':
	    KEY( "RegKey",	ch->registration_key,	fread_number( fp ) );
	    KEY( "RegLogins",	ch->registration_logins,fread_number( fp ) );
	    KEY( "EmailSet",	ch->email_set,		fread_number( fp ) );
	    KEY( "Registerred",	ch->registerred,	fread_number( fp ) );
	    KEY( "RealName", ch->pcdata->real_name, fread_string( fp ) );
	    KEY( "ResFlags", ch->res_flags, fread_number( fp ) );
	    KEY( "Rage", ch->rage, fread_number( fp ) );
	    KEY( "Race",        ch->race,       
				race_lookup(fread_string( fp )) );
	    if ( !str_cmp( word, "Realm") )
	    {
		ch->realm          = get_realm(fread_string( fp ));
		if (ch->realm == NULL) ch->realm = prime_realm;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "RaceVal" ) )
	    {
		ch->value[fread_number( fp )] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( char_realm(ch), fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( prime_realm, ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "Sanity", 	ch->pcdata->sanity,	fread_number( fp ) );
	    KEY( "Sapphires", 	ch->sapphires,		fread_number( fp ) );
	    KEY( "Souls",       ch->pcdata->souls,      fread_number( fp ) );
	    KEY( "SavingThrow", ch->saving_throw,       fread_number( fp ) );
	    KEY( "Save",        ch->saving_throw,       fread_number( fp ) );
	    KEY( "Scro",        ch->lines,              fread_number( fp ) );
	    KEY( "Sex",         ch->sex,                fread_number( fp ) );
	    KEY( "ShortDescr",  ch->short_descr,        fread_string( fp ) );
	    KEY( "ShD",         ch->short_descr,        fread_string( fp ) );
	    KEY( "Silv",        ch->silver,             fread_number( fp ) );
	    KEY( "Stance",      ch->stance,             fread_number( fp ) );
	    KEY( "StanceSkill0",ch->stance_skill[0],    fread_number( fp ) );
	    KEY( "StanceSkill1",ch->stance_skill[1],    fread_number( fp ) );
	    KEY( "StanceSkill2",ch->stance_skill[2],    fread_number( fp ) );
	    KEY( "StanceSkill3",ch->stance_skill[3],    fread_number( fp ) );
	    KEY( "StanceSkill4",ch->stance_skill[4],    fread_number( fp ) );
	    KEY( "StanceSkill5",ch->stance_skill[5],    fread_number( fp ) );
	    KEY( "StanceSkill6",ch->stance_skill[6],    fread_number( fp ) );
	    KEY( "StanceSkill7",ch->stance_skill[7],    fread_number( fp ) );
	    KEY( "StanceSkill8",ch->stance_skill[8],    fread_number( fp ) );
	    KEY( "StanceSkill9",ch->stance_skill[9],    fread_number( fp ) );
	    KEY( "StanceSkill10",ch->stance_skill[10],    fread_number( fp ) );
	    KEY( "StanceSkill11",ch->stance_skill[11],    fread_number( fp ) );
	    KEY( "StanceSkill12",ch->stance_skill[12],    fread_number( fp ) );
	    KEYS( "SwName",     ch->pcdata->switchname, fread_string( fp ) );
	    if ( !str_cmp( word, "Style"))
	    {
		int stat;
		for (stat = 0; stat < 8; stat++)
			ch->style_skill[stat] = fread_number(fp);
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "TrueSex",     ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "TSex",        ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trust",       ch->trust,              fread_number( fp ) );
	    KEY( "Tru",         ch->trust,              fread_number( fp ) );
	    KEY( "Tertiary",    ch->tertiaryrace,       fread_number( fp ) );
	    KEY( "TradeRes",	ch->trade_researching,  fread_number( fp ) );
	    KEY( "TradeTime",   ch->trade_time,		fread_number( fp ) );

 	    if ( !str_cmp( word, "Trade" ) )
	    {
		ch->trades[fread_number(fp)] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Tick_Timer" ) )
	    {
		ch->tick_timer[fread_number(fp)] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "TreeSkill" ) )
	    {
		ch->pcdata->treelearn[fread_number(fp)] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    
	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "TitlePre" ))
	    {
		ch->pcdata->title_prefix = fread_string( fp );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'U':
	    KEY ( "Upkeep",     ch->upkeep,             fread_number ( fp ) );

	case 'V':
	    KEY( "VulFlags",ch->vuln_flags,            fread_number ( fp ) );
	    KEY( "VoodooPoints",ch->pcdata->voodoo_points,            fread_number ( fp ) );
	    KEY( "Version",     ch->version,            fread_number ( fp ) );
	    KEY( "Vers",        ch->version,            fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Wimpy",       ch->wimpy,              fread_number( fp ) );
	    KEY( "Wimp",        ch->wimpy,              fread_number( fp ) );
	    KEY( "Wizn",        ch->wiznet,             fread_flag( fp ) );
	    break;

	case 'X':
	    KEY( "XHitroll",     ch->xhitroll,            fread_number( fp ) );
	    KEY( "XDamroll",     ch->xdamroll,            fread_number( fp ) );

	}

	if ( !fMatch )
	{
	    bug( "Fread_char: no match with:", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
	int vnum;
	
	vnum = fread_number(fp);
	if (get_mob_index(vnum) == NULL)
	{
	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
	else
	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
	bug("Fread_pet: no vnum in file.",0);
	pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
	word    = feof(fp) ? "END" : fread_word(fp);
	fMatch = FALSE;
	
	switch (UPPER(word[0]))
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol(fp);
	    break;
		
	case 'A':
	    KEY( "Act",         pet->act,               fread_flag(fp));
	    KEY( "AfBy",        pet->affected_by,       fread_flag(fp));
	    KEY( "Alig",        pet->alignment,         fread_number(fp));
	    
	    if (!str_cmp(word,"ACs"))
	    {
		int i;
		
		for (i = 0; i < 4; i++)
		    pet->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;
		
		paf = new_affect();
		
		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		   paf->type = sn;
		   
		paf->level      = fread_number(fp);
		paf->duration   = fread_number(fp);
		paf->modifier   = fread_number(fp);
		paf->location   = fread_number(fp);
		paf->bitvector  = fread_number(fp);
	 	paf->name	= fread_string( fp );
		if (!str_cmp(paf->name,"NOTHING"))
			free_string(paf->name);
	 	paf->end_message	= fread_string( fp );
		if (!str_cmp(paf->end_message,"NOTHING"))
			free_string(paf->end_message);
		paf->next       = pet->affected;
		pet->affected   = paf;
		fMatch          = TRUE;
		break;
	    }

	    if (!str_cmp(word,"Affc"))
	    {
		AFFECT_DATA *paf;
		int sn;
 
		paf = new_affect();
 
		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		   paf->type = sn;
 
		paf->where      = fread_number(fp);
		paf->level      = fread_number(fp);
		paf->duration   = fread_number(fp);
		paf->modifier   = fread_number(fp);
		paf->location   = fread_number(fp);
		paf->bitvector  = fread_number(fp);
	 	paf->name	= fread_string( fp );
		if (!str_cmp(paf->name,"NOTHING"))
		{
		    free_string(paf->name);
		    paf->name = NULL;
		}
	 	paf->end_message	= fread_string( fp );
		if (!str_cmp(paf->end_message,"NOTHING"))
		{
		    free_string(paf->end_message);
		    paf->end_message = NULL;
		}
		paf->next       = pet->affected;
		pet->affected   = paf;
		fMatch          = TRUE;
		break;
	    }
	     
	    if (!str_cmp(word,"AMod"))
	    {
		int stat;
		
		for (stat = 0; stat < MAX_STATS; stat++)
		    pet->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	     
	    if (!str_cmp(word,"Attr"))
	    {
		 int stat;
		 
		 for (stat = 0; stat < MAX_STATS; stat++)
		     pet->perm_stat[stat] = fread_number(fp);
		 fMatch = TRUE;
		 break;
	    }
	    break;
	     
	 case 'C':
	     KEY( "Clan",       pet->clan,       clan_lookup(fread_string(fp)));
	     KEY( "Comm",       pet->comm,              fread_flag(fp));
	     break;
	     
	 case 'D':
	     KEY( "Dam",        pet->damroll,           fread_number(fp));
	     KEY( "Desc",       pet->description,       fread_string(fp));
	     break;
	     
	 case 'E':
	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
		/* adjust hp mana move up  -- here for speed's sake */
		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
		{
		    percent = UMIN(percent,100);
		    pet->hit    += (pet->max_hit - pet->hit) * percent / 100;
		    pet->move   += (pet->max_move - pet->move)* percent / 100;
		}
		return;
	     }
	     KEY( "Exp",        pet->exp,               fread_number(fp));
	     break;
	     
	 case 'F':
	     KEY( "Flag2",       pet->flag2,              fread_number(fp));
	     KEY( "Flag3",       pet->flag3,              fread_number(fp));
	     KEY( "Flag4",       pet->flag4,              fread_number(fp));
	     KEY( "Flag5",       pet->flag5,              fread_number(fp));
	     KEY( "Flag6",       pet->flag6,              fread_number(fp));
	     break;

	 case 'G':
	     KEY( "Gold",       pet->gold,              fread_number(fp));
	     break;
	     
	 case 'H':
	     KEY( "Hit",        pet->hitroll,           fread_number(fp));
	     
	     if (!str_cmp(word,"HV"))
	     {
		pet->hit        = fread_number(fp);
		pet->max_hit    = fread_number(fp);
		pet->move       = fread_number(fp);
		pet->max_move   = fread_number(fp);
		fMatch = TRUE;
		break;
	     }
	     break;
	     
	case 'L':
	     KEY( "Levl",       pet->level,             fread_number(fp));
	     KEY( "LnD",        pet->long_descr,        fread_string(fp));
	     KEY( "LogO",       lastlogoff,             fread_number(fp));
	     break;
	     
	case 'M':
	     KEY( "MagRes",        pet->power[DISC_WARL_CONT],            fread_number( fp ) );
	     break;

	case 'N':
	     KEY( "Name",       pet->name,              fread_string(fp));
	     break;
	     
	case 'P':
	     KEY( "Pos",        pet->position,          fread_number(fp));
	     break;
	     
	case 'R':
	    KEY( "Race",        pet->race, race_lookup(fread_string(fp)));
	    break;
	    
	case 'S' :
	    KEY( "Save",        pet->saving_throw,      fread_number(fp));
	    KEY( "Sex",         pet->sex,               fread_number(fp));
	    KEY( "ShD",         pet->short_descr,       fread_string(fp));
	    KEY( "Silv",        pet->silver,            fread_number( fp ) );
	    break;
	    
	if ( !fMatch )
	{
	    bug("Fread_pet: no match.",0);
	    fread_to_eol(fp);
	}
	
	}
    }
}



void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */

    if (ch->version < 8)
    {
	/* eq wipe old pfile versions :)   MUHAHAHAHAHAHA */
	word   = feof( fp ) ? "End" : fread_word( fp );
	while (!str_cmp(word, "End"))
	{
	    word   = feof( fp ) ? "End" : fread_word( fp );
	}
	return;
    }

    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
	int vnum;
	first = FALSE;  /* fp will be in right place */
 
	vnum = fread_number( fp );
	if (  get_obj_index( vnum )  == NULL )
	{
	    bug( "Fread_obj: bad vnum %d.", vnum );
	}
	else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
	obj = new_obj();
	obj->name               = str_dup( "" );
	obj->short_descr        = str_dup( "" );
	obj->description        = str_dup( "" );
    }

    fNest               = FALSE;
    fVnum               = TRUE;
    iNest               = 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level      = fread_number( fp );
		paf->duration   = fread_number( fp );
		paf->modifier   = fread_number( fp );
		paf->location   = fread_number( fp );
		paf->bitvector  = fread_number( fp );
	 	paf->name	= fread_string( fp );
		if (!str_cmp(paf->name,"NOTHING"))
			free_string(paf->name);
	 	paf->end_message	= fread_string( fp );
		if (!str_cmp(paf->end_message,"NOTHING"))
			free_string(paf->end_message);
		paf->next       = obj->affected;
		obj->affected   = paf;
		fMatch          = TRUE;
		break;
	    }
	    if (!str_cmp(word,"Affc"))
	    {
		AFFECT_DATA *paf;
		int sn;
 
		paf = new_affect();
 
		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;
 
		paf->where      = fread_number( fp );
		paf->level      = fread_number( fp );
		paf->duration   = fread_number( fp );
		paf->modifier   = fread_number( fp );
		paf->location   = fread_number( fp );
		paf->bitvector  = fread_number( fp );
	 	paf->name	= fread_string( fp );
		if (!str_cmp(paf->name,"NOTHING"))
		{
		    free_string(paf->name);
		    paf->name = NULL;
		}
	 	paf->end_message	= fread_string( fp );
		if (!str_cmp(paf->end_message,"NOTHING"))
		{
		    free_string(paf->end_message);
		    paf->end_message = NULL;
		}
		paf->next       = obj->affected;
		obj->affected   = paf;
		fMatch          = TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Condition",   obj->condition,         fread_number( fp ) );
	    KEY( "Cost",        obj->cost,              fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description", obj->description,       fread_string( fp ) );
	    KEY( "Desc",        obj->description,       fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
		fMatch  = TRUE;
		break;
	    }

	    KEY( "ExtraFlags",  obj->extra_flags,       fread_number( fp ) );
	    KEY( "ExtF2",  obj->extra_flags2,       fread_number( fp ) );
	    KEY( "ExtF3",  obj->extra_flags3,       fread_number( fp ) );
	    KEY( "ExtF",        obj->extra_flags,       fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword             = fread_string( fp );
		ed->description         = fread_string( fp );
		ed->next                = obj->extra_descr;
		obj->extra_descr        = ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if (IS_OBJ_STAT3(obj, ITEM_INHABITED))
		{
		    bug("Destroying inhabited item.", 0);
		    return;
		}

		if ( !fNest || !fVnum || obj->pIndexData == NULL)
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj(obj);
		    return;
		}
		else
		{
		    if (!new_format)
		    {
			obj->next       = object_list;
			object_list     = obj;
			obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",    obj->item_type,         fread_number( fp ) );
	    KEY( "Ityp",        obj->item_type,         fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",       obj->level,             fread_number( fp ) );
	    KEY( "Lev",         obj->level,             fread_number( fp ) );
	    KEY( "LastOwner",   obj->last_owner, 	fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Material",   obj->material, 	fread_string( fp ) );
	    break;

	case 'N':
	    KEY( "Name",        obj->name,              fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;

	case 'P':		    
	    KEY( "PermSpell",   obj->perm_spell,        fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Size",        obj->size,	         fread_number( fp ) );
	    KEY( "ShortDescr",  obj->short_descr,       fread_string( fp ) );
	    KEY( "ShD",         obj->short_descr,       fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",       obj->timer,             fread_number( fp ) );
	    KEY( "Time",        obj->timer,             fread_number( fp ) );
	    break;

	case 'V':
	    KEY( "VoodooName",   obj->voodoo_name, 	fread_string( fp ) );
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]   = fread_number( fp );
		obj->value[1]   = fread_number( fp );
		obj->value[2]   = fread_number( fp );
		obj->value[3]   = fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch          = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0]   = fread_number( fp );
		obj->value[1]   = fread_number( fp );
		obj->value[2]   = fread_number( fp );
		obj->value[3]   = fread_number( fp );
		obj->value[4]   = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val2" ) )
	    {
		obj->value[5]   = fread_number( fp );
		obj->value[6]   = fread_number( fp );
		obj->value[7]   = fread_number( fp );
		obj->value[8]   = fread_number( fp );
		obj->value[9]   = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",   obj->wear_flags,        fread_number( fp ) );
	    KEY( "WeaF",        obj->wear_flags,        fread_number( fp ) );
	    KEY( "WearLoc",     obj->wear_loc,          fread_number( fp ) );
	    KEY( "Wear",        obj->wear_loc,          fread_number( fp ) );
	    KEY( "Weight",      obj->weight,            fread_number( fp ) );
	    KEY( "Wt",          obj->weight,            fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match with:", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
	}
    }
}


