/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "hell.h"
#include "db.h"
#include "lookup.h"


/* values for db2.c */
struct          social_type     social_table            [MAX_SOCIALS];
int             social_count;

extern AREA_DATA * area_last;

/* snarf a socials file */
void load_socials( FILE *fp)
{
    for ( ; ; ) 
    {
	struct social_type social;
	char *temp;
	/* clear social */
	social.char_no_arg = NULL;
	social.others_no_arg = NULL;
	social.char_found = NULL;
	social.others_found = NULL;
	social.vict_found = NULL; 
	social.char_not_found = NULL;
	social.char_auto = NULL;
	social.others_auto = NULL;

	temp = fread_word(fp);
	if (!strcmp(temp,"#0"))
	    return;  /* done */
#if defined(social_debug) 
	else 
	    fprintf(stderr,"%s\n\r",temp);
#endif

	strcpy(social.name,temp);
	fread_to_eol(fp);

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue; 
	}
	else
	    social.char_no_arg = temp;

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.others_no_arg = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.others_no_arg = temp;

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_found = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.char_found = temp;

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.others_found = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.others_found = temp; 

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.vict_found = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.vict_found = temp;

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_not_found = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.char_not_found = temp;

	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.char_auto = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.char_auto = temp;
	 
	temp = fread_string_eol(fp);
	if (!strcmp(temp,"$"))
	     social.others_auto = NULL;
	else if (!strcmp(temp,"#"))
	{
	     social_table[social_count] = social;
	     social_count++;
	     continue;
	}
	else
	    social.others_auto = temp; 
	
	social_table[social_count] = social;
	social_count++;
   }
   return;
}
    




/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char tmp;
    int group;
 
    for ( ; ; )
    {
	sh_int vnum;
	char letter;
	int iHash;
 
	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found.", 0 );
	    exit( 1 );
	}
 
	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;
 
	fBootDb = FALSE;
	if ( get_mob_index( vnum ) != NULL )
	{
	    bug( "Load_mobiles: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;
 
	pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
	pMobIndex->area			= area_last;
	pMobIndex->vnum                 = vnum;
	pMobIndex->new_format           = TRUE;
	newmobs++;
	pMobIndex->player_name          = fread_string( fp );
	pMobIndex->short_descr          = fread_string( fp );
	pMobIndex->long_descr           = fread_string( fp );
	pMobIndex->description          = fread_string( fp );
	pMobIndex->race                 = race_lookup(fread_string( fp ));
 
	pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
	pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
					| race_table[pMobIndex->race].act;
	pMobIndex->affected_by          = fread_flag( fp )
					| race_table[pMobIndex->race].aff;
	pMobIndex->alignment            = fread_number( fp );


	tmp = fread_letter( fp );
	if (tmp == 'S')
	{
	   group = vnum;
	}
	else
	{
	   ungetc(tmp,fp);
	   group = fread_number(fp);
	}
	pMobIndex->group                = group;

	pMobIndex->level                = fread_number( fp );
	pMobIndex->hitroll              = fread_number( fp );  

	/* read hit dice */
	pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
	/* 'd'          */                fread_letter( fp ); 
	pMobIndex->hit[DICE_TYPE]       = fread_number( fp );
	/* '+'          */                fread_letter( fp );   
	pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

	/* skip old mana poo */
	    fread_number( fp );
	    fread_letter( fp );
	    fread_number( fp );
	    fread_letter( fp );
	    fread_number( fp );

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]  = fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_TYPE]    = fread_number( fp );
					  fread_letter( fp );
	pMobIndex->damage[DICE_BONUS]   = fread_number( fp );
	pMobIndex->dam_type             = attack_lookup(fread_word(fp));

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]        = fread_number( fp ) * 10;
	pMobIndex->ac[AC_BASH]          = fread_number( fp ) * 10;
	pMobIndex->ac[AC_SLASH]         = fread_number( fp ) * 10;
	pMobIndex->ac[AC_EXOTIC]        = fread_number( fp ) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags            = fread_flag( fp ) 
					| race_table[pMobIndex->race].off;
	pMobIndex->imm_flags            = fread_flag( fp )
					| race_table[pMobIndex->race].imm;
	pMobIndex->res_flags            = fread_flag( fp )
					| race_table[pMobIndex->race].res;
	pMobIndex->vuln_flags           = fread_flag( fp )
					| race_table[pMobIndex->race].vuln;

	/* vital statistics */
	pMobIndex->start_pos            = position_lookup(fread_word(fp));
	pMobIndex->default_pos          = position_lookup(fread_word(fp));
	pMobIndex->sex                  = sex_lookup(fread_word(fp));

	pMobIndex->wealth               = fread_number( fp );

	pMobIndex->form                 = fread_flag( fp )
					| race_table[pMobIndex->race].form;
	pMobIndex->parts                = fread_flag( fp )
					| race_table[pMobIndex->race].parts;
	/* size */
	pMobIndex->size                 = size_lookup(fread_word(fp));
	pMobIndex->material             = str_dup(fread_word( fp ));

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if (letter == 'R')
	    {
		char *word;
		word = fread_word(fp);
		if (!str_prefix(word,"dragon"))
		    pMobIndex->secondaryrace = 1;
		else if (!str_prefix(word,"demon"))
		    pMobIndex->secondaryrace = 2;
		else if (!str_prefix(word,"daemon"))
		    pMobIndex->secondaryrace = 2;
		else if (!str_prefix(word,"vampire"))
		    pMobIndex->secondaryrace = 3;
		else if (!str_prefix(word,"were-wolf"))
		    pMobIndex->secondaryrace = 4;
		else if (!str_prefix(word,"lupine"))
		    pMobIndex->secondaryrace = 4;
		else if (!str_prefix(word,"eladrin"))
		    pMobIndex->secondaryrace = 5;
		else if (!str_prefix(word,"warlock"))
		    pMobIndex->secondaryrace = 6;
		else if (!str_prefix(word,"hero"))
		    pMobIndex->secondaryrace = 7;
		else
		{
		    bug("Secondary Race not found.",0);
		    exit(1);
		}
	    }
	    else if (letter == 'Z')
	    {
		char *word;
		word = fread_word(fp);
	
		if (!str_cmp(word, "poison_bite"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_POISON_BITE);
		else if (!str_cmp(word, "petrification"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_PETRIFICATION);
		else if (!str_cmp(word, "flame_claw"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_FLAME_CLAW);
		else if (!str_cmp(word, "web"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_WEB);
		else if (!str_cmp(word, "fire_spikes"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_FIRE_SPIKES);
		else if (!str_cmp(word, "freeze"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_FREEZE);
		else if (!str_cmp(word, "everliving"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_EVERLIVING);
		else if (!str_cmp(word, "split"))
		    SET_BIT(pMobIndex->mob_spec, SPEC_SPLIT);
		else
		{
		    bug("Z flag - special procedure not found", 0);
		    exit(1);
		}
 	    }
	    else if (letter == 'M')
	    {
		char *word;
		int p = -1, q;
		word = fread_word(fp);
	        for (q = 0 ; q < NUM_TRIGS ; q++)
	  	    if (!str_cmp(word, trigger[q])) p = q; 

	  	if (p == -1)
		{
		    bug("Mob_prog: Bad trigger",0);
		    exit( 1 );
		}

		word = fread_word(fp);
		if ((pMobIndex->trig[p] = get_prog_index( word ))==NULL)
		{
		    bug("Mob_prog: No such procedure",0);
		    exit( 1 );
		}
	    }
	    else if (letter == 'F')
	    {
		char *word;
		long vector;

		word                    = fread_word(fp);
		vector                  = fread_flag(fp);

		if (!str_prefix(word,"act"))
		    REMOVE_BIT(pMobIndex->act,vector);
		else if (!str_prefix(word,"aff"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"off"))
		    REMOVE_BIT(pMobIndex->off_flags,vector);
		else if (!str_prefix(word,"imm"))
		    REMOVE_BIT(pMobIndex->imm_flags,vector);
		else if (!str_prefix(word,"res"))
		    REMOVE_BIT(pMobIndex->res_flags,vector);
		else if (!str_prefix(word,"vul"))
		    REMOVE_BIT(pMobIndex->vuln_flags,vector);
		else if (!str_prefix(word,"for"))
		    REMOVE_BIT(pMobIndex->form,vector);
		else if (!str_prefix(word,"par"))
		    REMOVE_BIT(pMobIndex->parts,vector);
		else
		{
		    bug("Flag remove: flag not found.",0);
		    exit(1);
		}
	     }
	     else
	     {
		ungetc(letter,fp);
		break;
	     }
	}

	iHash                   = vnum % MAX_KEY_HASH;
	pMobIndex->next         = mob_index_hash[iHash];
	mob_index_hash[iHash]   = pMobIndex;
	top_mob_index++;
	kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

void add_mob( MOB_INDEX_DATA *mob )
{
    int iHash;

    iHash	= mob->vnum % MAX_KEY_HASH;
    mob->next	= mob_index_hash[iHash];
    mob_index_hash[iHash] = mob;
    top_mob_index++;
    kill_table[URANGE(0, mob->level, MAX_LEVEL-1)].number++;
}

void load_olcmobs( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
 
    for ( ; ; )
    {
	sh_int vnum;
	char letter;
	int iHash;
 
	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_olcmobiles: # not found.", 0 );
	    exit( 1 );
	}
 
	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;
 
	fBootDb = FALSE;
	if ( get_mob_index( vnum ) != NULL )
	{
	    bug( "Load_olcmobiles: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;
 
	pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
	pMobIndex->vnum                 = vnum;
	pMobIndex->area			= area_last;
	pMobIndex->new_format           = TRUE;
	newmobs++;
	pMobIndex->player_name          = fread_string( fp );
	pMobIndex->short_descr          = fread_string( fp );
	pMobIndex->long_descr           = fread_string( fp );
	pMobIndex->description          = fread_string( fp );
 
	pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
	pMobIndex->group		= fread_number( fp );
	pMobIndex->act			= fread_number( fp );
	pMobIndex->affected_by		= fread_number( fp );
	pMobIndex->alignment		= fread_number( fp );
	pMobIndex->level		= fread_number( fp );
	pMobIndex->hitroll		= fread_number( fp );
	pMobIndex->hit[0]		= fread_number( fp );
	pMobIndex->hit[1]		= fread_number( fp );
	pMobIndex->hit[2]		= fread_number( fp );
	pMobIndex->damage[0]		= fread_number( fp );
	pMobIndex->damage[1]		= fread_number( fp );
	pMobIndex->damage[2]		= fread_number( fp );
	pMobIndex->ac[0]		= fread_number( fp );
	pMobIndex->ac[1]		= fread_number( fp );
	pMobIndex->ac[2]		= fread_number( fp );
	pMobIndex->ac[3]		= fread_number( fp );
	pMobIndex->dam_type		= fread_number( fp );
	pMobIndex->off_flags		= fread_number( fp );
	pMobIndex->imm_flags		= fread_number( fp );
	pMobIndex->res_flags		= fread_number( fp );
	pMobIndex->vuln_flags		= fread_number( fp );
	pMobIndex->start_pos		= fread_number( fp );
	pMobIndex->default_pos		= fread_number( fp );
	pMobIndex->sex			= fread_number( fp );
	pMobIndex->race			= fread_number( fp );
	pMobIndex->wealth		= fread_number( fp );
	pMobIndex->form 		= fread_number( fp );
	pMobIndex->parts		= fread_number( fp );
	pMobIndex->mob_spec		= fread_number( fp );
	pMobIndex->size			= fread_number( fp );
	pMobIndex->material		= fread_string( fp );
	pMobIndex->body_form		= fread_number( fp );
	pMobIndex->secondaryrace	= fread_number( fp );
	pMobIndex->tertiaryrace		= fread_number( fp );
	
	pMobIndex->act			= 0;
	pMobIndex->affected_by		= 0;

	for ( ; ; )
 	{
	    letter			= fread_letter( fp );
	    if (letter == 'O')
	    {
		pMobIndex->mobolc_flags = fread_number( fp );
		pMobIndex->mobolc_flags2 = fread_number( fp );
	    }
	    else if (letter != '#')
 	    {
		if (letter == 'M')
		{
		    pMobIndex->trig[fread_number( fp )] = get_prog_index( fread_word(fp));
		}
	    }
	    else
	    {
	    	ungetc( letter, fp );
		break;
	    }
	}

	iHash                   = vnum % MAX_KEY_HASH;
	pMobIndex->next         = mob_index_hash[iHash];
	mob_index_hash[iHash]   = pMobIndex;
	top_mob_index++;
	kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}

MOB_PROG_DATA * get_prog_index( char *argument )
{
    MOB_PROG_DATA *mp;

    for (mp = mob_prog_list; mp != NULL; mp = mp->next)
    {
	if (!str_cmp(argument, mp->proc_name))
	    return mp;
    }

    return NULL;
}


/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
 
    for ( ; ; )
    {
	sh_int vnum;
	char letter;
	int iHash;
 
	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found.", 0 );
	    exit( 1 );
	}
 
	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;
 
	fBootDb = FALSE;
	if ( get_obj_index( vnum ) != NULL )
	{
	    bug( "Load_objects: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;
 
	pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );
	pObjIndex->vnum                 = vnum;
	pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num            = 0;
	newobjs++;
	pObjIndex->name                 = fread_string( fp );
	pObjIndex->short_descr          = fread_string( fp );
	pObjIndex->description          = fread_string( fp );
	pObjIndex->material             = fread_string( fp );
 
	pObjIndex->item_type            = item_lookup(fread_word( fp ));
	pObjIndex->extra_flags          = fread_flag( fp );
	pObjIndex->wear_flags           = fread_flag( fp );
	pObjIndex->artifact		= 0;
	pObjIndex->perm_spell		= -1;

	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]         = weapon_type(fread_word(fp));
	    pObjIndex->value[1]         = fread_number(fp);
	    pObjIndex->value[2]         = fread_number(fp);
	    pObjIndex->value[3]         = attack_lookup(fread_word(fp));
	    pObjIndex->value[4]         = fread_flag(fp);
	    break;
	case ITEM_CONTAINER:
	    pObjIndex->value[0]         = fread_number(fp);
	    pObjIndex->value[1]         = fread_flag(fp);
	    pObjIndex->value[2]         = fread_number(fp);
	    pObjIndex->value[3]         = fread_number(fp);
	    pObjIndex->value[4]         = fread_number(fp);
	    break;
	case ITEM_SPELL_BOOK:
	    pObjIndex->value[0]         = fread_number(fp); /* realm */
	    pObjIndex->value[1]         = fread_number(fp); /* protect levl */
	    pObjIndex->value[2]         = skill_lookup(fread_word(fp));
	    pObjIndex->value[3]         = skill_lookup(fread_word(fp));
	    pObjIndex->value[4]         = skill_lookup(fread_word(fp));
	    break;
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	    pObjIndex->value[0]         = fread_number(fp);
	    pObjIndex->value[1]         = fread_number(fp);
	    pObjIndex->value[2]         = liq_lookup(fread_word(fp));
	    pObjIndex->value[3]         = fread_number(fp);
	    pObjIndex->value[4]         = fread_number(fp);
	    break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]         = fread_number(fp);
	    pObjIndex->value[1]         = fread_number(fp);
	    pObjIndex->value[2]         = fread_number(fp);
	    pObjIndex->value[3]         = skill_lookup(fread_word(fp));
	    pObjIndex->value[4]         = fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
	    pObjIndex->value[0]         = fread_number(fp);
	    pObjIndex->value[1]         = skill_lookup(fread_word(fp));
	    pObjIndex->value[2]         = skill_lookup(fread_word(fp));
	    pObjIndex->value[3]         = skill_lookup(fread_word(fp));
	    pObjIndex->value[4]         = skill_lookup(fread_word(fp));
	    break;
	default:
	    pObjIndex->value[0]             = fread_flag( fp );
	    pObjIndex->value[1]             = fread_flag( fp );
	    pObjIndex->value[2]             = fread_flag( fp );
	    pObjIndex->value[3]             = fread_flag( fp );
	    pObjIndex->value[4]             = fread_flag( fp );
	    break;
	}
	pObjIndex->level                = fread_number( fp );
	pObjIndex->weight               = fread_number( fp );
	pObjIndex->cost                 = fread_number( fp ); 
	pObjIndex->spec_proc            = 0;

	/* condition */
	letter                          = fread_letter( fp );
	switch (letter)
	{
	    case ('P') :                pObjIndex->condition = 100; break;
	    case ('G') :                pObjIndex->condition =  90; break;
	    case ('A') :                pObjIndex->condition =  75; break;
	    case ('W') :                pObjIndex->condition =  50; break;
	    case ('D') :                pObjIndex->condition =  25; break;
	    case ('B') :                pObjIndex->condition =  10; break;
	    case ('R') :                pObjIndex->condition =   0; break;
	    default:                    pObjIndex->condition = 100; break;
	}

	/* Ok my little mateys... here is the silver weapons. */
	if (is_name("silver", pObjIndex->name)
	    && pObjIndex->item_type == ITEM_WEAPON)
	    SET_BIT(pObjIndex->value[4], WEAPON_SILVER);

 
	for ( ; ; )
	{
	    char letter;
 
	    letter = fread_letter( fp );
 
	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf;
 
		paf                     = alloc_perm( sizeof(*paf) );
		paf->where              = TO_OBJECT;
		paf->type               = -1;
		paf->level              = pObjIndex->level;
		paf->duration           = -1;
		paf->location           = fread_number( fp );
		paf->modifier           = fread_number( fp );
		paf->bitvector          = 0;
		paf->next               = pObjIndex->affected;
		pObjIndex->affected     = paf;
		top_affect++;
	    }

	    else if (letter == 'Q')
	    {
		pObjIndex->extra_flags2 = fread_flag( fp );
	    }

	    else if (letter == 'F')
	    {
		AFFECT_DATA *paf;
 
		paf                     = alloc_perm( sizeof(*paf) );
		letter                  = fread_letter(fp);
		switch (letter)
		{
		case 'A':
		    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		    paf->where          = TO_IMMUNE;
		    break;
		case 'R':
		    paf->where          = TO_RESIST;
		    break;
		case 'V':
		    paf->where          = TO_VULN;
		    break;
		default:
		    bug( "Load_objects: Bad where on flag set.", 0 );
		   exit( 1 );
		}
		paf->type               = -1;
		paf->level              = pObjIndex->level;
		paf->duration           = -1;
		paf->location           = fread_number(fp);
		paf->modifier           = fread_number(fp);
		paf->bitvector          = fread_flag(fp);
		paf->next               = pObjIndex->affected;
		pObjIndex->affected     = paf;
		top_affect++;
	    }

 
	    /* RELICS GO HERE - NOT AS GOOD AS ARTIFACTS */

	    else if ( letter == 'R' )
	    {
		char *word;
		word = fread_word(fp);
		SET_BIT(pObjIndex->extra_flags, ITEM_RELIC);

		if (!str_cmp(word, "wind_boots"))
		    pObjIndex->artifact = RELIC_WIND_BOOTS;
		else
		{
	 	    bug("Relic not found.", 0);
		    exit(1);
		}

            }


	    /* Add new artifacts here, all by Adam!!!!!!!!!!!!!!! */

	    else if ( letter == 'T' )
	    {
		char *word;
		word = fread_word(fp);
		SET_BIT(pObjIndex->extra_flags, ITEM_ARTIFACT);

		if (!str_cmp(word, "lodering"))
		    pObjIndex->artifact = ARTIFACT_LODERING;
		else if (!str_cmp(word, "armet"))
		    pObjIndex->artifact = ARTIFACT_ARMET;
		else if (!str_cmp(word, "sceptre_kharos"))
		    pObjIndex->artifact = ARTIFACT_KHAROS;
		else if (!str_cmp(word, "jewel_symal"))
		    pObjIndex->artifact = ARTIFACT_SYMAL;
		else if (!str_cmp(word, "jandors_ring"))
		{
		    pObjIndex->artifact = ARTIFACT_JANDOR;
		    SET_BIT(pObjIndex->extra_flags2, ITEM_KNOW_OWNER);
		}
		else if (!str_cmp(word, "staff_wizardry"))
		    pObjIndex->artifact = ARTIFACT_STAFF_WIZARDRY;
		else if (!str_cmp(word, "crown_of_ages"))
		    pObjIndex->artifact = ARTIFACT_CROWN_OF_AGES;
		else if (!str_cmp(word, "fire_opal"))
		    pObjIndex->artifact = ARTIFACT_FIRE_OPAL;
		else if (!str_cmp(word, "ptermian_scarab"))
		    pObjIndex->artifact = ARTIFACT_PTERMIAN_SCARAB;
		else if (!str_cmp(word, "bone_necklace"))
		    pObjIndex->artifact = ARTIFACT_NECKLACE_BONES;
		else if (!str_cmp(word, "eye_sensus"))
		    pObjIndex->artifact = ARTIFACT_EYE_SENSUS;
		else if (!str_cmp(word, "telecaster"))
		    pObjIndex->artifact = ARTIFACT_TELECASTER;
		else
		{
		    bug("Unknown artifact (T flag).", 0);
		    exit(1);
		}
            }
                
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;
 
		ed                      = alloc_perm( sizeof(*ed) );
		ed->keyword             = fread_string( fp );
		ed->description         = fread_string( fp );
		ed->next                = pObjIndex->extra_descr;
		pObjIndex->extra_descr  = ed;
		top_ed++;
	    }

	    else if ( letter == 'Z' )
	    {
		pObjIndex->spec_proc    = fread_number( fp );
	    }
 
	    else if (letter == 'P')
	    {
		char *word, ltr, *line;
		int trig=-1,ln;
		bool done = FALSE;

		word = fread_word(fp);
		if (!str_prefix(word, "get"))
		    trig = ITEM_TRIG_GET;
		else
		{
		    bug("ITEM_PROG:  bad trigger",0);
		    exit(1);
		}

		pObjIndex->trig[trig] = TRUE;
		ltr = fread_letter( fp );
		if (ltr != '{')
		{
		    bug("MOB_PROG:  '{' expected",0);
		    exit(1);
		}
		ln = 0;
		while (!done)
		{
		    ltr = fread_letter( fp );
		    if (ltr != '}')
		    {
			ungetc(ltr,fp);
			line = fread_string_eol( fp );
			while ( isspace(*line) )
			    line++;
			pObjIndex->prog[trig][ln] = str_dup(line);
			if (++ln >= IP_MAX_LINES)
			{
			    bug("ITEM_PROG:  too many lines before '}'",0);
			    exit(1);
			}
		    }
		    else done = TRUE;
		}
		for ( ;ln < IP_MAX_LINES; ln++)
		    pObjIndex->prog[trig][ln] = str_dup("break\n\r");

	    }
	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}
 
	iHash                   = vnum % MAX_KEY_HASH;
	pObjIndex->next         = obj_index_hash[iHash];
	obj_index_hash[iHash]   = pObjIndex;
	top_obj_index++;
    }
 
    return;
}



void load_olc_obj( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
 
    for ( ; ; )
    {
	sh_int vnum;
	char letter;
	int iHash;
 
	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found.", 0 );
	    exit( 1 );
	}
 
	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;
 
	fBootDb = FALSE;
	if ( get_obj_index( vnum ) != NULL )
	{
	    bug( "Load_objects: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;
 
	pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );
	pObjIndex->vnum                 = vnum;
	pObjIndex->new_format           = TRUE;
	pObjIndex->reset_num            = 0;
	newobjs++;
	pObjIndex->name                 = fread_string( fp );
	pObjIndex->short_descr          = fread_string( fp );
	pObjIndex->description          = fread_string( fp );
	pObjIndex->material             = fread_string( fp );
 
  	pObjIndex->perm_spell		= -1;
	pObjIndex->item_type            = fread_number( fp );
	pObjIndex->extra_flags          = fread_flag( fp );
	pObjIndex->extra_flags2         = 0;
	pObjIndex->wear_flags           = fread_flag( fp );

	pObjIndex->value[1]         = fread_number(fp);
	pObjIndex->value[2]         = fread_number(fp);
	pObjIndex->value[3]         = fread_number(fp);
	pObjIndex->value[4]         = fread_number(fp);
	pObjIndex->value[5]         = fread_number(fp);

	pObjIndex->level                = fread_number( fp );
	pObjIndex->weight               = fread_number( fp );
	pObjIndex->cost                 = fread_number( fp ); 
	pObjIndex->spec_proc            = 0;


        /* Ok my little mateys... here is the silver weapons. */
        if (is_name("silver", pObjIndex->name)
            && pObjIndex->item_type == ITEM_WEAPON)
            SET_BIT(pObjIndex->value[4], WEAPON_SILVER);


	/* condition */
	pObjIndex->condition = fread_number( fp );

	for ( ; ; )
	{
	    char letter;
 
	    letter = fread_letter( fp );
 
	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf;
 
		paf                     = alloc_perm( sizeof(*paf) );
		paf->where              = TO_OBJECT;
		paf->type               = -1;
		paf->level              = pObjIndex->level;
		paf->duration           = -1;
		paf->location           = fread_number( fp );
		paf->modifier           = fread_number( fp );
		paf->bitvector          = 0;
		paf->next               = pObjIndex->affected;
		pObjIndex->affected     = paf;
		top_affect++;
	    }

	    else if (letter == 'Q')
	    {
		pObjIndex->extra_flags2 = fread_flag( fp );
	    }

	    else if (letter == 'S')
	    {
		pObjIndex->perm_spell = fread_number( fp );
	    }

	    else if (letter == 'F')
	    {
		AFFECT_DATA *paf;
 
		paf                     = alloc_perm( sizeof(*paf) );
		letter                  = fread_letter(fp);
		switch (letter)
		{
		case 'A':
		    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		    paf->where          = TO_IMMUNE;
		    break;
		case 'R':
		    paf->where          = TO_RESIST;
		    break;
		case 'V':
		    paf->where          = TO_VULN;
		    break;
		default:
		    bug( "Load_objects: Bad where on flag set.", 0 );
		   exit( 1 );
		}
		paf->type               = -1;
		paf->level              = pObjIndex->level;
		paf->duration           = -1;
		paf->location           = fread_number(fp);
		paf->modifier           = fread_number(fp);
		paf->bitvector          = fread_flag(fp);
		paf->next               = pObjIndex->affected;
		pObjIndex->affected     = paf;
		top_affect++;
	    }
 
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;
 
		ed                      = alloc_perm( sizeof(*ed) );
		ed->keyword             = fread_string( fp );
		ed->description         = fread_string( fp );
		ed->next                = pObjIndex->extra_descr;
		pObjIndex->extra_descr  = ed;
		top_ed++;
	    }

	    else if ( letter == 'Z' )
	    {
		pObjIndex->spec_proc    = fread_number( fp );
	    }
 
	    else if (letter == 'P')
	    {
		char *word, ltr, *line;
		int trig=-1,ln;
		bool done = FALSE;

		word = fread_word(fp);
		if (!str_prefix(word, "get"))
		    trig = ITEM_TRIG_GET;
		else
		{
		    bug("ITEM_PROG:  bad trigger",0);
		    exit(1);
		}

		pObjIndex->trig[trig] = TRUE;
		ltr = fread_letter( fp );
		if (ltr != '{')
		{
		    bug("MOB_PROG:  '{' expected",0);
		    exit(1);
		}
		ln = 0;
		while (!done)
		{
		    ltr = fread_letter( fp );
		    if (ltr != '}')
		    {
			ungetc(ltr,fp);
			line = fread_string_eol( fp );
			while ( isspace(*line) )
			    line++;
			pObjIndex->prog[trig][ln] = str_dup(line);
			if (++ln >= IP_MAX_LINES)
			{
			    bug("ITEM_PROG:  too many lines before '}'",0);
			    exit(1);
			}
		    }
		    else done = TRUE;
		}
		for ( ;ln < IP_MAX_LINES; ln++)
		    pObjIndex->prog[trig][ln] = str_dup("break\n\r");

	    }
	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	iHash                   = vnum % MAX_KEY_HASH;
	pObjIndex->next         = obj_index_hash[iHash];
	obj_index_hash[iHash]   = pObjIndex;
	top_obj_index++;
    }
 
    return;
}


void add_obj( OBJ_INDEX_DATA *obj )
{
    int iHash;

    iHash	= obj->vnum % MAX_KEY_HASH;
    obj->next	= obj_index_hash[iHash];
    obj_index_hash[iHash] = obj;
    top_obj_index++;
}


