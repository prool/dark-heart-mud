/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish, 
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */
/*****************************************************************
* Dark Heart coding functions				  	 *
* (C) 1997 Chris Mellish of Mellsoft				 *
* Dedicated to Pandora of God Wars as she's a miserable, ugly,   *
* wench who we all hate and love to torment.                     *
*****************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "tables.h"

extern const char * discipline[MAX_DISCIPLINES];

CHAR_DATA * animate_object( OBJ_DATA *obj )
{
    CHAR_DATA *mob;
    int level;
    int i;
    char buf[MAX_STRING_LENGTH];

    if (obj->in_room == NULL) return NULL;
    if (obj->sw_char != NULL) return NULL;

    level = obj->level;
    level += obj->size * 10;

    mob = create_blobling();

    if (mob == NULL)
    {
	bug("create_blobling failed.", 0);
	return NULL;
    }

    char_to_room( mob, obj->in_room, obj->altitude );
    obj_from_room( obj );
    obj_to_room(obj, get_room_index(prime_realm, 47), 0);

    mob->made_from		= obj;
    obj->animated		= mob;

    WRITE_STR( mob->name,  	  obj->name );
    WRITE_STR( mob->short_descr,  obj->short_descr );
    sprintf(buf, "%s is here.\n\r", obj->short_descr);
    buf[0] = UPPER(buf[0]);
    WRITE_STR( mob->long_descr,   buf );
    WRITE_STR( mob->description,  obj->description );
    mob->level 			= level;
    mob->max_hit		= obj->size * (50 + dice(1, 100)) + level * 2;
    mob->hit			= mob->max_hit * obj->condition / 100;
    mob->damage[DICE_NUMBER]	= 1;
    mob->damage[DICE_TYPE]	= (obj->size + 1) * 10;
    mob->dam_type		= DAM_BASH;
    mob->imm_flags		= IMM_POISON | IMM_CHARM | IMM_DROWNING | IMM_MENTAL | IMM_HOLY | IMM_NEGATIVE;
    mob->res_flags		= RES_LIGHT | RES_COLD | RES_BASH;
    mob->vuln_flags		= VULN_SOUND;
    mob->start_pos		= POS_STANDING;
    mob->default_pos		= POS_STANDING;
    mob->sex			= number_range(1, 2);
    mob->secondaryrace		= 0;
    mob->race			= RACE_HUMAN;
    mob->form			= FORM_CONSTRUCT;
    mob->parts			= PART_ARMS | PART_LEGS | PART_HANDS | PART_FEET | PART_EYESTALKS;
    mob->size			= obj->size;
    WRITE_STR( mob->material,     obj->material );
    mob->gold			= 0;
    mob->silver			= 0;
    mob->alignment		= 0;

    /* Item affect flags */
    switch ( obj->perm_spell )
    {
	case ITEMA_SANCTUARY :
	    SET_BIT(mob->affected_by, AFF_SANCTUARY);
	    break;

	case ITEMA_FLY :
	    SET_BIT(mob->affected_by, AFF_FLYING);
	    break;

	case ITEMA_HASTE :
	    SET_BIT(mob->affected_by, AFF_HASTE);
	    break;

	case ITEMA_INVISIBILITY :
	    SET_BIT(mob->affected_by, AFF_INVISIBLE);
	    break;

	case ITEMA_ARMOR :
	case ITEMA_SHIELD :
	case ITEMA_IRON_SKIN :
 	case ITEMA_WINDTOMB :
	    for ( i = 0 ; i < 4 ; i++)
		mob->armor[i] -= 30; 	   
 	    break;

	case ITEMA_ACCURACY :
	    mob->hitroll += 20;	  	
    }

    return mob;
}

void save_clan_leaders()
{
    FILE *fp;
    int clan;

    if ((fp = fopen("clan.leaders", "w")) == NULL)
    {
	bug("Error saving clan leaders.  Couldn't open file", 0);
	return;
    }

    for (clan = 0; clan < MAX_CLAN; clan++)
    {
	if (clan_table[clan].name == NULL) break;

	fprintf( fp, "%d\n%s~\n%s~\n%d %s~\n%d %d\n\n",
		clan,
		clan_data[clan].name,
		clan_data[clan].leader,
		clan_data[clan].members,
		clan_data[clan].description,
		clan_data[clan].fighting, 
		clan_data[clan].ally); 
    }

    fprintf( fp, "-1 -1 -1 -1" );

    fclose( fp );
}

void load_clan_leaders()
{
    FILE *fp;
    int clan;

    if ((fp = fopen("clan.leaders", "r")) == NULL)
    {
	bug("Error loading clan leaders.  Couldn't open file", 0);
	return;
    }

    for (clan = 0; clan < MAX_CLAN; clan++)
    {
	if (clan_table[clan].name == NULL) break;

	clan_data[clan].name		= clan_table[clan].name;
	clan_data[clan].leader		= "<none>";
	clan_data[clan].members		= 0;
	clan_data[clan].description	= "<none>";
	clan_data[clan].fighting	= -1;
	clan_data[clan].ally		= -1;
    }

    for ( ; ; )
    {
	clan = fread_number( fp );

	if (clan == -1)
		break;
	clan_data[clan].name		= fread_string( fp );
	clan_data[clan].leader		= fread_string( fp );
	clan_data[clan].members		= fread_number( fp );
	clan_data[clan].description	= fread_string( fp );
	clan_data[clan].fighting	= fread_number( fp );
	clan_data[clan].ally		= fread_number( fp );
	if (clan_data[clan].name == NULL)
	    clan_data[clan].name		= clan_table[clan].name;
	if (clan_data[clan].leader == NULL)
	    clan_data[clan].leader		= "<none>";
	if (clan_data[clan].description == NULL)
	    clan_data[clan].description	= "<none>";
    }

    fclose( fp );
}

char * clan_fighting( int clan )
{
    if (clan_data[clan].fighting < 0 || clan_data[clan].fighting >= MAX_CLAN)
	return "no-one";
    if (clan_data[clan].fighting == 0)
	return "all mortals";
    return clan_table[clan_data[clan].fighting].name;
}

char * clan_ally( int clan )
{
    if (clan_data[clan].ally < 0 || clan_data[clan].ally >= MAX_CLAN)
	return "no-one";
    if (clan_data[clan].ally == 0)
	return "all clans";
    return clan_table[clan_data[clan].ally].name;
}

void do_buydiscipline(CHAR_DATA *ch, char *argument)
{
    int cost, i, chosen = -1;

    if (ch->secondaryrace == 0) return;

    if (disciplines_known(ch) > 5)
    {
	stc("You may not learn any more disciplines.\n\r", ch);
	return;
    }

    cost = (disciplines_known(ch) - 3) * 50 + 100;
    if (cost < 100) cost = 100;

    if (ch->qp < cost)
    {
	stcf(ch, "You need %d more kai point%s to learn a discipline.\n\r",
		(cost - ch->qp), ((cost - ch->qp) == 1) ? "" : "s");
	return;
    }

    reset_learnable_disciplines(ch, ch->secondaryrace);

    if (argument[0] == 0)
    {
	stcf(ch, "For %d kai points you may learn any of the following:\n\r", cost);
	for ( i = 1; i < MAX_DISCIPLINES; i++)
	{
	    if (ch->power[i] == -1)
		stcf(ch, "    %s\n\r", discipline[i]);
	}
	stc("Buydiscipline <name> to buy the discipline.\n\r", ch);
	return;
    }

    for (i = 1; i < MAX_DISCIPLINES; i++)
    {
	if (!str_prefix(argument, discipline[i]) && ch->power[i] == -1)
	    chosen = i;
    }

    if (chosen == -1)
    {
	stc("You know of no such discipline.\n\r", ch);
	return;
    }

    ch->qp -= cost;
    stc("As you inner energy streams towards the skys in a blast of white light,\n\rknowledge is passed back from the gods into your mind.\n\r", ch);
    act("A brilliant beam of white lights shoots from $n's body\n\rtowards the sky.", ch, NULL, NULL, TO_ROOM);
    ch->power[chosen] = 0;
    save_char_obj(ch);
}

void do_berserk(CHAR_DATA *ch, char *argument)
{
    bool attacked = FALSE;
    int sk = get_skill(ch, gsn_berserk), atk, i, itv;
    CHAR_DATA *vch, *vch_next;

    for (i = 0; i < 5; i++)
	ch->initiative[i] = 0;

    for (vch = ch->in_room->people;  vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (ch == vch || dice(1, 100) >= sk) continue;

	atk = dice(1, 3) - 1;
	if (atk == 0 && (BODY_PART(ch, BODY_RIGHT_ARM) || BODY_PART(ch, BODY_RIGHT_HAND))) atk = 1;
	if (atk == 1 && (BODY_PART(ch, BODY_LEFT_ARM) || BODY_PART(ch, BODY_LEFT_HAND))) atk = 4;

	if (atk == 0) itv = initiative_cost(vch, get_eq_char(vch, WEAR_RIGHT_HAND));
	else if (atk == 1) itv = initiative_cost(vch, get_eq_char(vch, WEAR_LEFT_HAND));
	else { atk = 4; itv = initiative_cost(vch, NULL); }

	if (!attacked)
	{
	    act("$n screams with rage and charges at $N.", ch, NULL, vch, TO_NOTVICT);
	    act("$n screams with rage and charges at you.", ch, NULL, vch, TO_VICT);
	    act("You scream with rage and charges at $N.", ch, NULL, vch, TO_CHAR);
	}
	else
	{
	    act("In $s blind fury, $n turns and charges at $N.", ch, NULL, vch, TO_NOTVICT);
	    act("In $s blind fury, $n turns and charges at you.", ch, NULL, vch, TO_VICT);
	    act("In your blind fury, you turn and charges at $N.", ch, NULL, vch, TO_CHAR);
	}
	ch->fighting = vch;
	ch->initiative[atk] = 1;
	ch->distance = 0;
	vch->distance = 0;
	SET_BIT(ch->flag6, AFF6_AUTO_ATTACK);
	char_attack( ch );
	REMOVE_BIT(ch->flag6, AFF6_AUTO_ATTACK);
	ch->distance = des_dist(vch);
	vch->distance = ch->distance;
	ch->initiative[atk] = 0;
	attacked = TRUE;
    }
    if (!attacked)
    {
	act("$n screams with rage.", ch, NULL, NULL, TO_ROOM);
	stc("You scream with rage.", ch);
    }
    WAIT_STATE(ch, 30);
}
