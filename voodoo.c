/******************************************************************
 * VOODOO.C -- (C) 1996, by Adam Powell                           *
 * Misc. functions coded by AdAm PoWeLl                           *
 *   yeah man, let dat black magic run in yer veins               *
 ******************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hell.h"
#include "magic.h"

DECLARE_DO_FUN(do_look	);

/* Local functions required */
int 	  count_juju_bags    args( (CHAR_DATA *ch) );
OBJ_DATA  *get_voodoo_doll   args( (CHAR_DATA *ch, char *nam) );
CHAR_DATA *get_char_doll     args( (CHAR_DATA *ch, char *nam) );
bool      extract_ingredient args( (CHAR_DATA *ch, sh_int ingredient) );
OBJ_DATA  *find_ingredient   args( (CHAR_DATA *ch, sh_int ingredient) );
void      show_list_to_char  args( ( OBJ_DATA *list, CHAR_DATA *ch,  
                                    bool fShort, bool fShowNothing ) ); 
void      show_char_to_char  args( (CHAR_DATA *list, CHAR_DATA *ch) );
 
void do_ritual (CHAR_DATA *ch, char *argument)
{
}

int count_ingredients (CHAR_DATA *ch, int ingredient)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;

    int count = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_CONTAINER)
	{
	    for (obj2 = obj->contains ; obj2 != NULL ; obj2 = obj2->next_content)
	    {
                if ( IS_OBJ_STAT2(obj2, ingredient ) )
	    	    count++;
	    }
	}	    
        if ( IS_OBJ_STAT2(obj, ingredient ) )
	    count++;
    }
    return count;
}

void do_scalp (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
	send_to_char("Who do you wish to scalp?\n\r", ch);
	return;
    }

    if ( (victim = get_char_room(ch, argument) ) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position > POS_INCAP)
    {
	send_to_char("You don't want to risk waking them up.\n\r", ch);
	return;
    }

    if (!TIME_UP(victim, TIMER_SCALPED))
    {
	send_to_char("The hair has already been torn from their pate.\n\r", ch);
	return;
    }

    WAIT_STATE(ch, 10);

    SET_TIMER(victim, TIMER_SCALPED, 24);

    act("$n bends down and grabs a few hairs from $N's head."
	    , ch, NULL, victim, TO_NOTVICT);
    act("You reach down and rip some hairs out of $N's head.",
	    ch, NULL, victim, TO_CHAR);

    obj = create_object(get_obj_index(12000), 0); 
    SET_BIT(obj->extra_flags2, ITEM_VOO_HEAD);

    sprintf(buf, obj->short_descr, GET_PROPER_NAME(victim) );
    free_string(obj->short_descr);
    obj->short_descr = str_dup(buf);

    sprintf(buf, obj->name, GET_PROPER_NAME(victim) );
    free_string(obj->name);
    obj->name = str_dup(buf);

    free_string(obj->voodoo_name);
    obj->voodoo_name = str_dup(GET_PROPER_NAME(victim));

    obj_to_char(obj, ch);
}


void do_shake (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL;
    OBJ_DATA *obj2;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool dead = FALSE;
    bool head = FALSE;
    bool thread = FALSE;
    bool body = FALSE;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( arg1[0] == '\0' )
    {
        send_to_char( "You shake your head.\n\r", ch );
        act("$n shakes $s head.", ch, NULL, NULL, TO_ROOM );
        return;
    }
       
    if ( (obj = get_obj_eq(ch, arg1, ch) ) == NULL )
    {
	send_to_char("You are not carrying that item.\n\r", ch);
	return;
    }

    if (IS_OBJ_STAT2(obj, ITEM_JUJU_BAG)
	&& arg2[0] != '\0'
	&& ch->pcdata->voodoo_points > 10 )
    {
	ch->pcdata->voodoo_points -= 10;
	for(obj2 = obj->contains ; obj2 != NULL ; obj2 = obj2->next_content)
	{
	    if (IS_OBJ_STAT2(obj2, ITEM_VOO_HEAD)
		&& obj2->voodoo_name
		&& !str_cmp(obj2->voodoo_name, arg2))
	    {
	        head = TRUE;
		act("$p glows.", ch, obj2, NULL, TO_CHAR);
		continue;
	    }
	    if (IS_OBJ_STAT2(obj2, ITEM_VOO_DEAD)
		&& obj2->voodoo_name
		&& !str_cmp(obj2->voodoo_name, arg2))
	    {
	        dead = TRUE;
		act("$p start to hum.", ch, obj2, NULL, TO_CHAR);
		continue;
	    }
	    if (IS_OBJ_STAT2(obj2, ITEM_VOO_THREAD)
		&& obj2->voodoo_name
		&& !str_cmp(obj2->voodoo_name, arg2))
	    {
	        thread = TRUE;
		act("$p vibrate suddenly.", ch, obj2, NULL, TO_CHAR);
		continue;
	    }
	    if (IS_OBJ_STAT2(obj2, ITEM_VOO_BODY)
		&& obj2->voodoo_name
		&& !str_cmp(obj2->voodoo_name, arg2))
	    {
	        body = TRUE;
		act("$p starts to shudder uncontrollably.", ch, obj2, NULL, TO_CHAR);
		continue;
	    }
	}
	if (!body || !head || !dead || !thread)
	{
	    send_to_char("You have not got all the ingredients necessary.\n\r", ch);
	    return;
	}
	extract_obj(obj);

	send_to_char("The juju bag explodes in a cloud of smoke, and a small doll appears.\n\r", ch);
	act("$n's juju bag vanishes, and a doll appears in $s hands.", ch, NULL, NULL, TO_ROOM);
	
	obj = create_object(get_obj_index(12012), 0);
	obj->item_type = ITEM_VOODOO_DOLL;

	arg2[0] = UPPER(arg2[0]);
	obj->value[0] = 0;

	sprintf(buf, obj->short_descr, arg2 );
	free_string(obj->short_descr);
	obj->short_descr = str_dup(buf);

	sprintf(buf, obj->name, arg2 );
	free_string(obj->name);
	obj->name = str_dup(buf);

	free_string(obj->voodoo_name);
	obj->voodoo_name = str_dup(arg2);
	obj_to_char(obj, ch);
	return;
    }

    act("You shake $p around, but to no avail.", ch, obj, NULL, TO_CHAR);
    act("$n starts to shake $p for some reason.", ch, obj, NULL, TO_ROOM);

}

CHAR_DATA * get_char_doll (CHAR_DATA *ch, char *nam)
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING && d->character != NULL
            && can_see(ch, d->character)
	    && !str_cmp(GET_PROPER_NAME(d->character), nam) )
	    return d->character;
    }

    return NULL;
}

OBJ_DATA *get_voodoo_doll (CHAR_DATA *ch, char *nam)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( nam, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( obj->item_type == ITEM_VOODOO_DOLL
        &&   (can_see_obj( ch, obj ) )
	&&   !str_cmp(nam, obj->voodoo_name) )
        {
            if ( ++count == number )
                return obj;
        }
    }

    return NULL;
}


bool extract_ingredient(CHAR_DATA *ch, sh_int ingredient)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_CONTAINER)
	{
	    for (obj2 = obj->contains ; obj2 != NULL ; obj2 = obj2->next_content)
	    {
                if ( IS_OBJ_STAT2(obj2, ingredient ) )
	        {
		    extract_obj(obj);
		    return TRUE;
		}
	    }
	}	    
	else if ( IS_OBJ_STAT2(obj, ingredient) )
	{
	    extract_obj(obj);
	    return TRUE;
	}
    }

    return FALSE;
}

OBJ_DATA *find_ingredient(CHAR_DATA *ch, sh_int ingredient)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_CONTAINER)
	{
	    for (obj2 = obj->contains ; obj2 != NULL ; obj2 = obj2->next_content)
	    {
                if ( IS_OBJ_STAT2(obj2, ingredient ) )
		    return obj2;
	    }
	}	    
	else if ( IS_OBJ_STAT2(obj, ingredient) )
	{
	    return obj;
	}
    }

    return FALSE;
}


void do_voodoo (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char buf[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);

    if (IS_NPC(ch))
    {
	send_to_char("You don't possess any voodoo powers in this form.\n\r", ch);
	return;
    }

    if (ch->alignment > -300 && ch->level != 120)
    {
	send_to_char("You would never attempt such an evil thing.\n\r", ch);
	return;
    }

    if (voodoo_level(ch) == 0)
    {
	send_to_char("But you don't know any voodoo yet.\n\r", ch);
	return;
    }

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax : voodoo [help ingredients bags create stab scry", ch);
	if (voodoo_level(ch) > 1)
	    send_to_char("\n                 hex animate tear puppet beckon", ch);
	if (voodoo_level(ch) > 2)
	    send_to_char("\n                 curse tarot evileye", ch);
	if (ch->level == 120)
	    send_to_char(" immdoll", ch);
	send_to_char("]\n\r", ch);
	return;
    }

    if (!str_prefix(arg1, "evileye"))
    {
	OBJ_DATA *obj;
  	bool gotdoll = FALSE;

	if (voodoo_level(ch) < 3)
	{
	    do_voodoo(ch, "");
	    return;
	}

	if (argument[0] == '\0')
	{
	    stc("Syntax : voodoo evileye <message>\n\r", ch);
	    return;
	}

	if (bad_to_interpret(argument))
	{
	    stc("That is not a good idea!\n\r", ch);
	    return;
	}

        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        {
            if ( obj->item_type == ITEM_VOODOO_DOLL
            &&   (can_see_obj( ch, obj ) )
	    &&   !str_cmp(GET_PROPER_NAME(ch), obj->voodoo_name) )
	    {
		gotdoll = TRUE;	
	    }
        }

	if (!gotdoll)
	{
	    sprintf(buf, "You aren't carrying a doll of yourself!\n\r");
	    send_to_char(buf, ch);
	    return;
	}

 	if (ch->pcdata->voodoo_points < 6)
	{
	    stc("You need six voodoo points to set an evileye.\n\r", ch);
	    return;
	}
	
 	ch->pcdata->voodoo_points -= 6;
	WRITE_STR(ch->pcdata->evileye, argument);
	WAIT_STATE(ch, 12);
	stc("You have set a new evileye contingency.\n\r", ch);
	return;
    }


    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (!str_prefix(arg1, "create"))
    {
	if (ch->pcdata->voodoo_points < 3)
	{
	    send_to_char("You need three voodoo points to make a juju bag.\n\r", ch);
	    return;
	}

	ch->pcdata->voodoo_points -= 3;

	send_to_char("You snap your fingers and a little black juju bag appears.\n\r", ch);
	act("$n snaps $s fingers and a little juju bag appears out of mid air.", ch, NULL, NULL, TO_ROOM); 

	obj = create_object(get_obj_index(12001), 0);

	sprintf(buf, obj->name, GET_PROPER_NAME(ch) );
	free_string(obj->name);
	obj->name = str_dup(buf);

	sprintf(buf, obj->short_descr, GET_PROPER_NAME(ch) );
	free_string(obj->short_descr);
	obj->short_descr = str_dup(buf);
	obj_to_char(obj, ch);
	SET_BIT(obj->extra_flags2, ITEM_JUJU_BAG);
	return;
    }

    if (!str_prefix(arg1, "immdoll") && ch->level == 120)
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : voodoo immdoll <name>\n\r", ch);
	    return;
	}

	if (!str_cmp("Blathersnot", arg2))
	{
	    arg2[0] = '\0';
	    sprintf(arg2, "Jander.  NOT");
	}

	obj = create_object(get_obj_index(12012), 0);
	obj->item_type = ITEM_VOODOO_DOLL;

	arg2[0] = UPPER(arg2[0]);

	sprintf(buf, obj->short_descr, arg2 );
	free_string(obj->short_descr);
	obj->short_descr = str_dup(buf);

	sprintf(buf, obj->name, arg2 );
	free_string(obj->name);
	obj->name = str_dup(buf);

	free_string(obj->voodoo_name);
	obj->voodoo_name = str_dup(arg2);
	obj_to_char(obj, ch);
	obj->value[0] = 0;
	return;
    }

    if (!str_prefix(arg1, "bags"))
    {
	bool bagitem = FALSE;
	int bagno = 0;
	OBJ_DATA *obj2;

	if (count_juju_bags(ch) == 0)
	{
	    send_to_char("You are not carrying any juju bags.\n\r", ch);
	    return;
	}
	
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        {
	    if (IS_OBJ_STAT2(obj, ITEM_JUJU_BAG) && obj->item_type == ITEM_CONTAINER)
	    {
		bagitem = FALSE;

		sprintf(buf, "JuJu bag number %d...\n\r", ++bagno);
		send_to_char(buf, ch);
	        for (obj2 = obj->contains ; obj2 != NULL ; obj2 = obj2->next_content)
	        {
                    if ( IS_OBJ_STAT2(obj2, ITEM_VOO_HEAD ))
	    	    {
			bagitem = TRUE;
			sprintf(buf, "    contains %s  (something of the head)\n\r", obj2->short_descr);
			send_to_char(buf,ch);
		    }
                    else if ( IS_OBJ_STAT2(obj2, ITEM_VOO_DEAD ))
	    	    {
			bagitem = TRUE;
			sprintf(buf, "    contains %s  (something of the dead)\n\r", obj2->short_descr);
			send_to_char(buf,ch);
		    }
                    else if ( IS_OBJ_STAT2(obj2, ITEM_VOO_BODY ))
	    	    {
			bagitem = TRUE;
			sprintf(buf, "    contains %s  (something of the body)\n\r", obj2->short_descr);
			send_to_char(buf,ch);
		    }
                    else if ( IS_OBJ_STAT2(obj2, ITEM_VOO_THREAD ))
	    	    {
			bagitem = TRUE;
			sprintf(buf, "    contains %s  (something of the thread)\n\r", obj2->short_descr);
			send_to_char(buf,ch);
		    }
		}
	    if (!bagitem)
		send_to_char("    contains no voodoo doll parts.\n\r", ch);
	    }
	}	    
	return;
    }

    if (!str_prefix(arg1, "help"))
    {
	banner_to_char(banner_name("voodoo"), ch);
	sprintf(buf, "You have %d voodoo points.", ch->pcdata->voodoo_points);
	cent_to_char(buf, ch);
	sprintf(buf, "You are currently holding %d juju bags.",
	    count_juju_bags(ch) );
	cent_to_char(buf, ch);
	if (ch->pcdata->evileye != NULL)
 	{
 	    sprintf(buf, "Your evileye contingency is %s",
		ch->pcdata->evileye);
	    ctc(buf, ch);
	}
	sprintf(buf, "Your current voodoo ranking is :");
	if (voodoo_level(ch) == 1)
	    strcat(buf, " delver.");
	else if (voodoo_level(ch) == 2 && ch->pcdata->true_sex == SEX_MALE)
	    strcat(buf, " gypsy.");
	else if (voodoo_level(ch) == 2)
	    strcat(buf, " fortune teller.");
	else if (voodoo_level(ch) == 3 && ch->pcdata->true_sex == SEX_MALE)
	    strcat(buf, " voodoo master.");
	else if (voodoo_level(ch) == 3)
	    strcat(buf, " witch.");
	cent_to_char(buf, ch);
	divide4_to_char(ch);
	return;
    }

    if (!str_prefix(arg1, "ingredients") )
    {
	banner_to_char(banner_name("voodoo"), ch);
	sprintf(buf, "Cloves of Bajuju : %d",
	    count_ingredients(ch, ITEM_INGRED_BAJUJU) );
	cent_to_char(buf, ch);
	sprintf(buf, "Muldaleaves : %d",
	    count_ingredients(ch, ITEM_INGRED_MULDALEAF) );
	cent_to_char(buf, ch);
	sprintf(buf, "Bunches of Wormwart : %d",
	    count_ingredients(ch, ITEM_INGRED_WORMWART) );
	cent_to_char(buf, ch);
	sprintf(buf, "Handfuls of Tillifreen : %d",
	    count_ingredients(ch, ITEM_INGRED_TILLIFREEN) );
	cent_to_char(buf, ch);
	sprintf(buf, "Bunches of Scullyweed : %d",
	    count_ingredients(ch, ITEM_INGRED_SCULLYWEED) );
	cent_to_char(buf, ch);
	sprintf(buf, "Pinches of Frogbreath : %d",
	    count_ingredients(ch, ITEM_INGRED_FROGBREATH) );
	cent_to_char(buf, ch);
	sprintf(buf, "Handfuls of Papacoco : %d",
	    count_ingredients(ch, ITEM_INGRED_PAPACOCO) );
	cent_to_char(buf, ch);
	sprintf(buf, "Lumps of Gloin : %d",
	    count_ingredients(ch, ITEM_INGRED_GLOIN) );
	cent_to_char(buf, ch);

	divide4_to_char(ch);
	return;
    }

    if (!str_prefix(arg1, "scry"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : voodoo scry <victim>\n\r", ch);
	    return;
 	}
	
	if ((obj = get_voodoo_doll(ch, arg2)) == NULL)
	{
	    sprintf(buf, "You aren't carrying a doll of %s.\n\r", arg2);
	    send_to_char(buf, ch);
	    return;
	}

	if (ch->pcdata->voodoo_points < 2)
	{
	    send_to_char("This ability costs two voodoo points.\n\r", ch);
	    return;
	}

	if (!extract_ingredient(ch, ITEM_INGRED_FROGBREATH))
	{
	    send_to_char("You lack the proper ingredient.\n\r", ch);
	    return;
	}

	victim = get_char_doll(ch, arg2); 
	ch->pcdata->voodoo_points -= 2;
	act("You spread the leaf upon $p.", ch, obj, NULL, TO_CHAR);
	act("$n spreads leaves upon $p.", ch, obj, NULL, TO_ROOM);

	if (victim == NULL ||  victim->in_room == NULL)
	{
	    send_to_char("You cannot locate your victim.\n\r", ch);
	    return;
	}
        send_to_char( victim->in_room->name, ch );
        send_to_char( "\n\r",ch);
	send_to_char( victim->in_room->description, ch );
	show_list_to_char( victim->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( victim->in_room->people,   ch );
	return;
    }

    if (!str_prefix(arg1, "hex"))
    {
	if (voodoo_level(ch) < 2)
	{
	    do_voodoo(ch, "");
	    return;
	}

	if (arg2[0] == '\0' || arg3[0] == '\0')
	{
	    send_to_char("Syntax : voodoo hex <victim> {hex}\n\r", ch);
	    send_to_char("   type 'help voodoo hex' for a list of hexes\n\r", ch);
	    return;
 	}

	if ((obj = get_voodoo_doll(ch, arg2)) == NULL)
	{
	    sprintf(buf, "You aren't carrying a doll of %s.\n\r", arg2);
	    send_to_char(buf, ch);
	    return;
	}
    }

    if (!str_prefix(arg1, "puppet"))
    {
	if (voodoo_level(ch) < 2)
	{
	    do_voodoo(ch, "");
	    return;
	}

	if (arg2[0] == '\0' || arg3[0] == '\0')
	{
	    send_to_char("Syntax : voodoo puppet <victim> <social>\n\r", ch);
	    return;
 	}

	if ((obj = get_voodoo_doll(ch, arg2)) == NULL)
	{
	    sprintf(buf, "You aren't carrying a doll of %s.\n\r", arg2);
	    send_to_char(buf, ch);
	    return;
	}

	if (ch->pcdata->voodoo_points < 1)
	{
	    send_to_char("You need a voodoo point for this.\n\r", ch);
	    return;
	}

	ch->pcdata->voodoo_points -= 1;
	act("You play with $p as a puppet.", ch, obj, NULL, TO_CHAR);
	act("$n plays with $p as a puppet.", ch, obj, NULL, TO_ROOM);
	WAIT_STATE(ch, 6);

	victim = get_char_doll(ch, arg2);
	if (victim == NULL) return;

	check_social(victim, arg3, argument);
	return;
    }

    if (!str_prefix(arg1, "beckon"))
    {
	if (voodoo_level(ch) < 2)
	{
	    do_voodoo(ch, "");
	    return;
	}

	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : voodoo beckon <victim>\n\r", ch);
	    return;
 	}

	if ((obj = get_voodoo_doll(ch, arg2)) == NULL)
	{
	    sprintf(buf, "You aren't carrying a doll of %s.\n\r", arg2);
	    send_to_char(buf, ch);
	    return;
	}

	if (ch->pcdata->voodoo_points < 10)
	{
	    send_to_char("You need ten voodoo points to beckon a victim.\n\r", ch);
	    return;
	}

	if (!extract_ingredient(ch, ITEM_INGRED_WORMWART))
	{
	    send_to_char("You lack the proper ingredient.\n\r", ch);
	    return;
	}

	send_to_char("The wormwart vanishes into thin air.\n\r", ch);

	ch->pcdata->voodoo_points -= 10;
	WAIT_STATE(ch, 12);
	victim = get_char_doll(ch, arg2); 

	if (victim == NULL)
	{
	    send_to_char("Nothing happens.\n\r", ch);
	    return;
	}

	char_from_room(victim);
	char_to_room(victim, ch->in_room, ch->altitude);
	act("$n appears in a cloud of smoke.", victim, NULL, NULL, TO_ROOM);
	send_to_char("You have been teleported.\n\r", victim);
	do_look(victim, "auto");
	return;
    }

    if (!str_prefix(arg1, "tear"))
    {
	bool arm = FALSE, leg = FALSE;

	if (voodoo_level(ch) < 2)
	{
 	    do_voodoo(ch, "");
	    return;
	}

	if (arg2[0] == '\0' || arg3[0] == '\0')
	{
	    send_to_char("Syntax : voodoo tear <victim> {leg arm}\n\r", ch);
	    return;
 	}

	if ((obj = get_voodoo_doll(ch, arg2)) == NULL)
	{
	    sprintf(buf, "You aren't carrying a doll of %s.\n\r", arg2);
	    send_to_char(buf, ch);
	    return;
	}

	if (!str_prefix(arg3, "leg")) leg = TRUE;
	else if (!str_prefix(arg3, "arm")) arm = TRUE;
	else
	{
	    send_to_char("You may only tear off an arm or a leg.\n\r", ch);
	    return;
	}

	if (ch->pcdata->voodoo_points < 20)
	{
	    send_to_char("You need twenty voodoo points to effectively tear up a doll.\n\r", ch);
	    return;
	}

	ch->pcdata->voodoo_points -= 20;

	victim = get_char_doll(ch, arg2); 
	WAIT_STATE(ch, 12);

	if (victim != NULL && victim->in_room == NULL)
	{
	    send_to_char("The limb will not budge.\n\r", ch);
	    return;
	}

	if (arm && !IS_SET(obj->value[0], DOLL_LEFT_ARM) )
	{
	    WAIT_STATE(ch, 12);
	    send_to_char("You rip off the doll's left arm.\n\r", ch);
	    act("$n rips the left arm off $p.", ch, obj, NULL, TO_ROOM);
	    SET_BIT(obj->value[0], DOLL_LEFT_ARM);

	    if (victim != NULL && !BODY_PART(victim, BODY_LEFT_ARM) )
	    {
		send_to_char("There is a crunching noise as your left arm is ripped out of its socket.\n\r", victim);
		lose_body_part(victim, BODY_LEFT_ARM);
		act("$n's left arm is suddenly ripped from its socket.", victim, NULL, NULL, TO_ROOM);
	    }
	    return;
	}
	else if (arm && !IS_SET(obj->value[0], DOLL_RIGHT_ARM) )
	{
	    WAIT_STATE(ch, 12);
	    send_to_char("You rip off the doll's right arm.\n\r", ch);
	    act("$n rips the right arm off $p.", ch, obj, NULL, TO_ROOM);
	    SET_BIT(obj->value[0], DOLL_RIGHT_ARM);

	    if (victim != NULL && !BODY_PART(victim, BODY_RIGHT_ARM) )
	    {
		send_to_char("There is a crunching noise as your right arm is ripped out of its socket.\n\r", victim);
		lose_body_part(victim, BODY_RIGHT_ARM);
		act("$n's right arm is suddenly ripped from its socket.", victim, NULL, NULL, TO_ROOM);
	    }
	    return;
	}
	else if (arm)
	{
	    act("$p has no more arms to tear off.", ch, obj, NULL, TO_CHAR);
	    return;
	}

	if (leg && !IS_SET(obj->value[0], DOLL_LEFT_LEG) )
	{
	    WAIT_STATE(ch, 12);
	    send_to_char("You rip off the doll's left leg.\n\r", ch);
	    act("$n rips the left leg off $p.", ch, obj, NULL, TO_ROOM);
	    SET_BIT(obj->value[0], DOLL_LEFT_LEG);

	    if (victim != NULL && !BODY_PART(victim, BODY_LEFT_LEG) )
	    {
		send_to_char("There is a crunching noise as your left leg is ripped out of its socket.\n\r", victim);
		lose_body_part(victim, BODY_LEFT_LEG);
		act("$n's left leg is suddenly ripped from its socket.", victim, NULL, NULL, TO_ROOM);
	    }
	    return;
	}
	else if (leg && !IS_SET(obj->value[0], DOLL_RIGHT_LEG) )
	{
	    WAIT_STATE(ch, 12);
	    send_to_char("You rip off the doll's right leg.\n\r", ch);
	    act("$n rips the right leg off $p.", ch, obj, NULL, TO_ROOM);
	    SET_BIT(obj->value[0], DOLL_RIGHT_LEG);

	    if (victim != NULL && !BODY_PART(victim, BODY_RIGHT_LEG) )
	    {
		send_to_char("There is a crunching noise as your right leg is ripped out of its socket.\n\r", victim);
		lose_body_part(victim, BODY_RIGHT_LEG);
		act("$n's right leg is suddenly ripped from its socket.", victim, NULL, NULL, TO_ROOM);
	    }
	    return;
	}
	else if (leg)
	{
	    act("$p has no more legs to tear off.", ch, obj, NULL, TO_CHAR);
	    return;
	}
    }

    if (!str_prefix(arg1, "stab"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("Syntax : voodoo stab <victim>\n\r", ch);
	    return;
	}

	if ((obj = get_voodoo_doll(ch, arg2)) == NULL)
	{
	    sprintf(buf, "You aren't carrying a doll of %s.\n\r", arg2);
	    send_to_char(buf, ch);
	    return;
	}
	if (ch->pcdata->voodoo_points < 5)
	{
	    send_to_char("You need five voodoo points to effectively stab a doll.\n\r", ch);
	    return;
	}
	if ((victim = get_char_doll(ch, arg2)) == NULL)
	{
	    act("You repeatedly stab $p, to no avail.", ch, obj, NULL, TO_CHAR);
	    WAIT_STATE(ch, 7);
	    act("$n stabs $p repeatedly.", ch, obj, NULL, TO_ROOM);
	    return;
	}
	ch->pcdata->voodoo_points -= 5;
	WAIT_STATE(ch, 8);
	act("You repeatedly stab $p.", ch, obj, NULL, TO_CHAR);
	act("$n stabs $p repeatedly.", ch, obj, NULL, TO_ROOM);
	send_to_char("You feel a sharp pain in your chest and drop to the floor.\n\r", victim);
	act("$n clutches $s chest and drops to the floor.", victim, NULL, NULL, TO_ROOM);
	damage_old(ch, victim, dice(4, ch->generation+2), 0, DAM_PIERCE, FALSE);
	if (victim->fighting == NULL
	    && victim->position > POS_RESTING)
		 victim->position = POS_SITTING;
	return;
    }

    do_voodoo(ch, "");
    return;
}

int count_juju_bags (CHAR_DATA *ch)
{
   OBJ_DATA *obj;

   int count = 0;
   for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( IS_OBJ_STAT2(obj, ITEM_JUJU_BAG ) )
	    count++;
    }

    return count;
}


int voodoo_level (CHAR_DATA *ch)
{
    if (ch->level == 120) return 3;

    if (ch->secondaryrace == 0
	&& ch->tertiaryrace == 0)
	return 0;

    else if (ch->tertiaryrace == TERTIARY_WITCH)
	return 3;

    else if (ch->secondaryrace == RACE_VAMPIRE
	|| ch->secondaryrace == RACE_DAEMON)
    	return 2;

    else return 1;
}
