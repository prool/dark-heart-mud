/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hell.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split         );
DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_yell          );
DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_wake          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_listhouse     );
DECLARE_DO_FUN(do_buyhouse      );
DECLARE_DO_FUN(do_quit          );


/* special procedures */
DECLARE_DO_FUN(do_activate      );
DECLARE_DO_FUN(item_spec_1      );
DECLARE_DO_FUN(item_spec_2      );
DECLARE_DO_FUN(item_spec_100    );



/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool    sremove_obj      args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void    wear_obj        args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace,bool show ) ); CD *    find_keeper     args( (CHAR_DATA *ch ) );
int     get_cost        args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void    obj_to_keeper   args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
int	free_hand	args( (CHAR_DATA *ch) );
int	free_hold_slot	args( (CHAR_DATA *ch) );
OD *    get_obj_keeper  args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));
void    check_grow_arms args( (CHAR_DATA *ch) );
bool    can_get_money   args( (CHAR_DATA *ch, int coins) );
bool    can_get_gems    args( (CHAR_DATA *ch, int gems) );
bool    is_wrong_size   args( (CHAR_DATA *ch, OBJ_DATA *obj) );

#undef OD
#undef  CD

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
	if (!str_cmp(wch->name,obj->owner))
	    owner = wch;

    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}


void outfit(CHAR_DATA *ch)
{
    OBJ_DATA *obj;

    /* Give a backpack to start with */
    if (dice(1, 2) == 1)
        obj = create_object( get_obj_index ( 801 ), 0 );
    else
        obj = create_object( get_obj_index ( 813 ), 0 );

    obj_to_char(obj, ch);


    /* now dish out supplies and food */
    obj = create_object( get_obj_index ( 814 ), 0 );
    obj_to_char(obj, ch);

    obj = create_object( get_obj_index ( 810 ), 0 );
    obj_to_char(obj, ch);
    obj = create_object( get_obj_index ( 810 ), 0 );
    obj_to_char(obj, ch);

    if (ch->race == RACE_ELF)
    {
	obj = create_object( get_obj_index ( 809 ), 0 );
    	obj_to_char(obj, ch);
	if (dice(1, 2) == 1)
	{    
    	    obj = create_object( get_obj_index ( 810 ), 0 );
    	    obj_to_char(obj, ch);
	}
	if (dice(1, 2) == 1)
	{    
    	    obj = create_object( get_obj_index ( 810 ), 0 );
    	    obj_to_char(obj, ch);
	}
    }

    obj = create_object( get_obj_index ( 811 ), 0 );
    obj_to_char(obj, ch);

    if (dice(1,2) == 1)
    {
        obj = create_object( get_obj_index ( 812 ), 0 );
        obj_to_char(obj, ch);
    }
    
    /* Now do the boots  - i added a new procedure to make it easy */
    if (ch->race == RACE_ELF)
	make_item(ch, 822, WEAR_FEET);
    else if (ch->race == RACE_GNOME)
	make_item(ch, 821, WEAR_FEET);
    else if (ch->sex ==  SEX_MALE)
	make_item(ch, 820, WEAR_FEET);
    else make_item (ch, 804, WEAR_FEET);

    /* Now the rings - isnt that procedure very good!!!!! */
    if (ch->sex == SEX_FEMALE)
	make_item(ch, 807+dice(1,2), WEAR_FINGER_L);

    /* now the bracers/bracelets */
    if (ch->sex == SEX_MALE && ch->race != RACE_GNOME)
	make_item(ch, 818, WEAR_WRIST_R);
    else if (ch->sex == SEX_FEMALE && ch->race == RACE_ELF)
	make_item(ch, 823, WEAR_WRIST_L);
    else if (ch->race == RACE_GNOME)
	make_item(ch, 824, WEAR_WRIST_L);

    /* now the fun bit, the clothes */
    if (ch->sex == SEX_FEMALE && ch->race != RACE_GNOME)
    {
	if (dice(1,3) == 1)
	    make_item(ch,802,WEAR_BODY);
	else if (dice(1,2) == 1)
	    make_item(ch,816,WEAR_BODY);
	else make_item(ch,819,WEAR_BODY);
    }
    else if (ch->race == RACE_GNOME) make_item(ch,825,WEAR_BODY);
    else
    { 
        if (dice(1,3) == 1)
            make_item(ch,815,WEAR_BODY);
        else make_item(ch,819,WEAR_BODY);
    } 

    /* now head gear - only for elves really... */
    if (ch->race == RACE_ELF) make_item(ch,803,WEAR_HEAD);

    /* now the weapons */
    make_item(ch, 805, WEAR_RIGHT_HAND);
    if (get_size(ch) >= SIZE_MEDIUM) make_item(ch, 806, WEAR_LEFT_HAND);
	
}

void make_item(CHAR_DATA *ch, int vnum, int equip)
{
    OBJ_DATA *obj;
    obj = create_object( get_obj_index ( vnum ), 0 );
    obj_to_char(obj, ch);
    obj->size = get_size(ch);
    if (equip != 666)
        equip_char(ch, obj, equip);
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }

    if (is_wearing_artifact(ch, ARTIFACT_SYMAL) != NULL
	|| IS_INSUBSTANTIAL(ch))
    {
	act("Your hand passes straight through $p.", ch, obj, NULL, TO_CHAR);
	act("$n tries to get $p, but $s hand passes straight through it.", ch, obj, NULL, TO_ROOM);
	return;
    }

    if (obj->item_type == ITEM_CASH)
    {
	if (obj->value[1] < CASH_EMERALDS
	    && !can_get_money(ch, obj->value[0]) )
	{
	    stc("You cannot carry that many coins.\n\r", ch);
	    return;
	}
	else if (!can_get_gems(ch, obj->value[0]))
	{
	    stc("You cannot carry that many gems.\n\r", ch);
	    return;
	}
	if (obj->value[1] == CASH_COPPER)
	    ch->copper += obj->value[0];
	else if (obj->value[1] == CASH_SILVER)
	    ch->silver += obj->value[0];
	else if (obj->value[1] == CASH_GOLD)
	    ch->gold += obj->value[0];
	else if (obj->value[1] == CASH_EMERALDS)
	    ch->emeralds += obj->value[0];
	else if (obj->value[1] == CASH_SAPPHIRES)
	    ch->sapphires += obj->value[0];
	else if (obj->value[1] == CASH_DIAMONDS)
	    ch->diamonds += obj->value[0];
	
	extract_obj(obj);
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
    &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
	    if (gch->on == obj )
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
	}
    }
		
    if ( container != NULL )
    {
	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
	    obj->timer = 0;     
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	obj_from_obj( obj );
    }
    else if (ch->cur_form == FRM_WHIRLWIND)
    {
	act( "You pick up $p and sweep it round the room.", ch, obj, container, TO_CHAR );
	act( "$n picks up $p and sweeps it round the room.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }
    else if (IS_SET(ch->flag2, AFF_CLAW) )
    {
	act( "You pick up $p in your claws.", ch, obj, container, TO_CHAR);
	act( "$n picks up $p in $s claws.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }
    else if (form_data[ch->cur_form].max_items == 1)
    {
	act( "You pick up $p in your mouth.", ch, obj, container, TO_CHAR );
	act( "$n picks up $p in $s mouth.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }
    else
    {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	act( "$n gets $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->silver += obj->value[0];
	ch->gold += obj->value[1];
	if (IS_SET(ch->act,PLR_AUTOSPLIT))
	{ /* AUTOSPLIT code */
	  members = 0;
	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	  {
	    if (gch->altitude != ch->altitude) continue;
	    if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
	      members++;
	  }

	  if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
	  {
	    sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	    do_split(ch,buffer);        
	  }
	}
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    return;
}

bool can_get_money(CHAR_DATA *ch, int coins)
{
    if (get_carry_weight(ch) + coins / 10 > can_carry_w(ch) )
	return FALSE;
    return TRUE;
}

bool can_get_gems(CHAR_DATA *ch, int gems)
{
    if (get_carry_weight(ch) + gems > can_carry_w(ch) )
	return FALSE;
    return TRUE;
}

void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    if (has_no_arms(ch))
    {	
	stc("You have no arms and cannot pick up items.\n\r", ch);
	return;
    }

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );

	    if (obj == NULL && ch->in_room->diamonds == 0
		&& ch->in_room->emeralds == 0
		&& ch->in_room->sapphires == 0
		&& ch->in_room->gold == 0
		&& ch->in_room->silver == 0
		&& ch->in_room->copper == 0)
	    {
		send_to_char("There is nothing to pick up here.\n\r", ch);
		return;
	    }

	    if  (obj == NULL && ((!str_prefix(arg1, "diamonds") && ch->in_room->diamonds == 0)
		  || (!str_prefix(arg1, "emeralds") && ch->in_room->emeralds == 0)
		  || (!str_prefix(arg1, "sapphires") && ch->in_room->sapphires == 0)
		  || (!str_prefix(arg1, "gold") && ch->in_room->gold == 0)
		  || (!str_prefix(arg1, "silver") && ch->in_room->silver == 0)
		  || (!str_prefix(arg1, "copper") && ch->in_room->copper == 0)))
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    if (obj != NULL) get_obj( ch, obj, NULL );

	    if (obj == NULL && !str_prefix(arg1, "diamonds") && ch->in_room->diamonds > 0)
            {
		if (!can_get_gems(ch, 1))
		{
	 	    send_to_char("You can't carry it!\n\r", ch);
		    return;
		}
		send_to_char("You pick up a diamond.\n\r", ch);
		act("$n picks up a diamond from the floor.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->diamonds -= 1;
		ch->diamonds += 1;
		return;
    	    }
    	    else if (obj == NULL && !str_prefix(arg1, "sapphires") && ch->in_room->sapphires > 0)
    	    {
		if (!can_get_gems(ch, 1))
		{
	 	    send_to_char("You can't carry it!\n\r", ch);
		    return;
		}
		send_to_char("You pick up a sapphire.\n\r", ch);
		act("$n picks up a sapphire from the floor.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->sapphires -= 1;
		ch->sapphires += 1;
		return;
    	    }
    	    else if (obj == NULL && !str_prefix(arg1, "emeralds") && ch->in_room->emeralds > 0)
    	    {
		if (!can_get_gems(ch, 1))
		{
	 	    send_to_char("You can't carry it!\n\r", ch);
		    return;
		}
		send_to_char("You pick up an emerald.\n\r", ch);
		act("$n picks up a emerald from the floor.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->emeralds -= 1;
		ch->emeralds += 1;
		return;
    	    }
    	    else if (obj == NULL && !str_prefix(arg1, "gold coin") && ch->in_room->gold > 0)
    	    {
		if (!can_get_money(ch, 1))
		{
	 	    send_to_char("You can't carry it!\n\r", ch);
		    return;
		}
		send_to_char("You pick up a gold coin.\n\r", ch);
		act("$n picks up a gold coin from the floor.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->gold -= 1;
		ch->gold += 1;
		return;
    	    }
    	    else if (obj == NULL && !str_prefix(arg1, "silver coin") && ch->in_room->silver > 0)
    	    {
		if (!can_get_money(ch, 1))
		{
	 	    send_to_char("You can't carry it!\n\r", ch);
		    return;
		}
		send_to_char("You pick up a silver coin.\n\r", ch);
		act("$n picks up a silver coin from the floor.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->silver -= 1;
		ch->silver += 1;
		return;
    	    }
    	    else if (obj == NULL && !str_prefix(arg1, "copper piece") && ch->in_room->copper > 0)
    	    {
		if (!can_get_money(ch, 1))
		{
	 	    send_to_char("You can't carry it!\n\r", ch);
		    return;
		}
		send_to_char("You pick up a copper piece.\n\r", ch);
		act("$n picks up a copper piece from the floor.", ch, NULL, NULL, TO_ROOM);
		ch->in_room->copper -= 1;
		ch->copper += 1;
		return;
            }
	    else if (obj == NULL)
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }
        }
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
		if (ch->in_room->copper > 0
			&& can_get_money(ch, ch->in_room->copper))
		{
		    sprintf(buf, "You pick up %ld copper pieces.\n\r", ch->in_room->copper);
		    send_to_char(buf, ch);
		    sprintf(buf, "$n picks up %ld copper pieces.", ch->in_room->copper);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		    ch->copper += ch->in_room->copper;
		    ch->in_room->copper = 0;
		    found = TRUE;
	 	}
		if (ch->in_room->silver > 0
			&& can_get_money(ch, ch->in_room->silver))
		{
		    sprintf(buf, "You pick up %ld silver coins.\n\r", ch->in_room->silver);
		    send_to_char(buf, ch);
		    sprintf(buf, "$n picks up %ld silver coins.", ch->in_room->silver);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		    ch->silver += ch->in_room->silver;
		    ch->in_room->silver = 0;
		    found = TRUE;
	 	}
		if (ch->in_room->gold > 0
			&& can_get_money(ch, ch->in_room->gold))
		{
		    sprintf(buf, "You pick up %ld gold coins.\n\r", ch->in_room->gold);
		    send_to_char(buf, ch);
		    sprintf(buf, "$n picks up %ld gold coins.", ch->in_room->gold);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		    ch->gold += ch->in_room->gold;
		    ch->in_room->gold = 0;
		    found = TRUE;
	 	}
		if (ch->in_room->emeralds > 0
			&& can_get_gems(ch, ch->in_room->emeralds))
		{
		    sprintf(buf, "You pick up %ld emeralds.\n\r", ch->in_room->emeralds);
		    send_to_char(buf, ch);
		    sprintf(buf, "$n picks up %ld emeralds.", ch->in_room->emeralds);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		    ch->emeralds += ch->in_room->emeralds;
		    ch->in_room->emeralds = 0;
		    found = TRUE;
	 	}
		if (ch->in_room->sapphires > 0
			&& can_get_gems(ch, ch->in_room->sapphires))
		{
		    sprintf(buf, "You pick up %ld sapphires.\n\r", ch->in_room->sapphires);
		    send_to_char(buf, ch);
		    sprintf(buf, "$n picks up %ld sapphires.", ch->in_room->sapphires);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		    ch->sapphires += ch->in_room->sapphires;
		    ch->in_room->sapphires = 0;
		    found = TRUE;
	 	}
		if (ch->in_room->diamonds > 0
			&& can_get_gems(ch, ch->in_room->diamonds))
		{
		    sprintf(buf, "You pick up %ld diamonds.\n\r", ch->in_room->diamonds);
		    send_to_char(buf, ch);
		    sprintf(buf, "$n picks up %ld diamonds.", ch->in_room->diamonds);
		    act(buf, ch, NULL, NULL, TO_ROOM);
		    ch->diamonds += ch->in_room->diamonds;
		    ch->in_room->diamonds = 0;
		    found = TRUE;
	 	}
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		}

	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER
	  && container->item_type != ITEM_QUIVER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( container->item_type == ITEM_QUIVER
	      && obj->item_type != ITEM_ARROW)
	{
	    stc("You can only put arrows or bolts into a quiver.\n\r", ch);
	    return;
  	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	if (WEIGHT_MULT(obj) != 100)
	{
	    send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
	    return;
	}

   	if (container->item_type == ITEM_CONTAINER)
  	{
	    if (get_obj_weight( obj ) + get_true_weight( container )
	         > (container->value[0] * 10) 
	       ||  get_obj_weight(obj) > (container->value[3] * 10))
	    {
	       send_to_char( "It won't fit.\n\r", ch );
	        return;
	    }
  	}

	if (container->pIndexData->vnum == OBJ_VNUM_PIT 
	&&  !CAN_WEAR(container,ITEM_TAKE))
	    if (obj->timer)
		SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	    else
		obj->timer = number_range(100,200);


	if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	    act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
	    act("You put $p on $P.",ch,obj,container, TO_CHAR);
	}
	else
	{
	    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	}
	obj_from_char( obj );
	obj_to_obj( obj, container );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   WEIGHT_MULT(obj) == 100
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_true_weight( container )
		 <= (container->value[0] * 10) 
	    &&   get_obj_weight(obj) < (container->value[3] * 10))
	    {
		/* No putting other things into quivers */
	  	if ( container->item_type == ITEM_QUIVER
	      	&& obj->item_type != ITEM_ARROW)
		{
	    	     stc("You can only put arrows or bolts into a quiver.\n\r", ch);
	   	     continue;
  		}

		if (container->pIndexData->vnum == OBJ_VNUM_PIT
		&&  !CAN_WEAR(obj, ITEM_TAKE) )
		    if (obj->timer)
			SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
		    else
			obj->timer = number_range(100,200);

		if (IS_SET(container->value[1],CONT_PUT_ON))
		{
		    act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
		    act("You put $p on $P.",ch,obj,container, TO_CHAR);
		}
		else
		{
		    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
		}

		obj_from_char( obj );
		obj_to_obj( obj, container );
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	char montype[100];
	/* 'drop NNNN coins' */
	int amount;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );

	if (arg[0] == '\0')
	{
	    send_to_char("Drop what?\n\r", ch);
	    return;
	}

	if ( amount <= 0 )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg, "piece") || !str_cmp(arg,"pieces") 
	||   !str_prefix( arg, "copper"))
	{
	    if (ch->copper < amount)
	    {
		send_to_char("You don't have that much copper.\n\r",ch);
		return;
	    }
	    if (amount > 1) sprintf(montype, "copper pieces");
	    else sprintf(montype, "copper piece");

	    ch->copper -= amount;
	    ch->in_room->copper += amount;
	}

	else if ( !str_prefix( arg, "emeralds"))
	{
	    if (ch->emeralds < amount)
	    {
		send_to_char("You don't have that many emeralds.\n\r",ch);
		return;
	    }
	    if (amount > 1) sprintf(montype, "emeralds");
	    else sprintf(montype, "emerald");

	    ch->emeralds -= amount;
	    ch->in_room->emeralds += amount;
	}
	else if ( !str_prefix( arg, "sapphires") )
	{
	    if (ch->sapphires < amount)
	    {
		send_to_char("You don't have that many sapphires.\n\r",ch);
		return;
	    }
	    if (amount > 1) sprintf(montype, "sapphires");
	    else sprintf(montype, "sapphire");

	    ch->sapphires -= amount;
	    ch->in_room->sapphires += amount;
	}
	else if ( !str_prefix( arg, "diamonds"))
	{
	    if (ch->diamonds < amount)
	    {
		send_to_char("You don't have that many diamonds.\n\r",ch);
		return;
	    }
	    if (amount > 1) sprintf(montype, "diamonds");
	    else sprintf(montype, "diamond");

	    ch->diamonds -= amount;
	    ch->in_room->diamonds += amount;
	}

	else if (!str_prefix(arg, "gold"))
	{
	    if (ch->gold < amount)
	    {
		send_to_char("You don't have that much gold.\n\r",ch);
		return;
	    }
	    if (amount > 1) sprintf(montype, "gold coins");
	    else sprintf(montype, "gold coin");

	    ch->gold -= amount;
	    ch->in_room->gold += amount;
	}
	else if (!str_prefix(arg, "silver") )
	{
	    if (ch->silver < amount)
	    {
		send_to_char("You don't have that much silver.\n\r",ch);
		return;
	    }
	    if (amount > 1) sprintf(montype, "silver coins");
	    else sprintf(montype, "silver coin");

	    ch->silver -= amount;
	    ch->in_room->silver += amount;
	}
	else 
	{
	    send_to_char("You can't do that.\n\r", ch);
	    return;
        } 
	if (amount > 1)
	    sprintf(buf, "You drop %d %s.\n\r", amount, montype);
	else
	    sprintf(buf, "You drop a single %s.\n\r",montype);
	send_to_char(buf, ch);

	if (amount > 1)
	    sprintf(buf, "$n drops %d %s.", amount, montype);
	else
	    sprintf(buf, "$n drops a single %s.",montype);
	act( buf, ch, NULL, NULL, TO_ROOM );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room, ch->altitude );
	free_string(obj->last_owner);
	obj->last_owner = str_dup(GET_PROPER_NAME(ch));

	if (!IS_IN_WATER(ch))
  	{
	    act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	    act( "You drop $p.", ch, obj, NULL, TO_CHAR );
	}
	else 
	{
	    act( "$n drops $p into the water.", ch, obj, NULL, TO_ROOM );
	    act( "You drop $p into the water.", ch, obj, NULL, TO_CHAR );
	}
	if (obj->item_type == ITEM_BAUBLE)
	    smash_bauble( ch, obj );

        else if (obj->item_type == ITEM_MAGIC_MIRROR)
	{
	    act("$p smashes on the ground.", ch, obj, NULL, TO_ROOM);
	    act("$p smashes on the ground.", ch, obj, NULL, TO_CHAR);
	    extract_obj(obj);
	}

	else if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
	{
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
	    extract_obj(obj);
	}
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room, ch->altitude );

		if (!IS_IN_WATER(ch))
	   	{
		    act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		    act( "You drop $p.", ch, obj, NULL, TO_CHAR );
		}
		else 
	   	{
		    act( "$n drops $p into the water.", ch, obj, NULL, TO_ROOM );
		    act( "You drop $p into the water.", ch, obj, NULL, TO_CHAR );
		}

		if (obj->item_type == ITEM_BAUBLE)
		    smash_bauble( ch, obj );
	        else if (obj->item_type == ITEM_MAGIC_MIRROR)
		{
		    act("$p smashes on the ground.", ch, obj, NULL, TO_ROOM);
		    act("$p smashes on the ground.", ch, obj, NULL, TO_CHAR);
		    extract_obj(obj);
		}
		else if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
		{
		    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
		    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
		    extract_obj(obj);
		}
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}

bool has_got_money( CHAR_DATA *ch, char *arg, int amount)
{
    if (!str_prefix(arg, "copper") && ch->copper < amount )
    {
	send_to_char("You don't have that much copper.\n\r", ch);
	return FALSE;
    }
    if (!str_prefix(arg, "silver") && ch->silver < amount )
    {
	send_to_char("You don't have that much silver.\n\r", ch);
	return FALSE;
    }
    if (!str_prefix(arg, "gold") && ch->gold < amount )
    {
	send_to_char("You don't have that much gold.\n\r", ch);
	return FALSE;
    }
    if (!str_prefix(arg, "emeralds") && ch->emeralds < amount )
    {
	send_to_char("You don't have that much emeralds.\n\r", ch);
	return FALSE;
    }
    if (!str_prefix(arg, "sapphires") && ch->sapphires < amount )
    {
	send_to_char("You don't have that much sapphires.\n\r", ch);
	return FALSE;
    }
    if (!str_prefix(arg, "diamonds") && ch->diamonds < amount )
    {
	send_to_char("You don't have that much diamonds.\n\r", ch);
	return FALSE;
    }
    return TRUE;
}

void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give the dosh to the victim' */
	int amount;
	char denom[20];

	amount   = atoi(arg1);
	argument = one_argument( argument, arg3 );

	if ( amount <= 0
	|| ( str_prefix( arg2, "copper") && str_prefix( arg2, "emeralds" ) && 
	     str_prefix( arg2, "gold") && str_prefix( arg2, "silver") &&
	     str_prefix( arg2, "diamonds") && str_prefix(arg2, "sapphires") ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}


	if ( arg3[0] == '\0' )
	{
	    send_to_char( "Give money to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg3 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( (!str_prefix(arg2, "copper") || !str_prefix(arg2, "gold")
		|| !str_prefix(arg2, "silver") )
		&& !can_get_money(victim, amount) )
	{
	    act("$N cannot carry that amount.", ch, NULL, victim, TO_CHAR);
	    act("$n tries to give you some coins, but they are too heavy.", ch, NULL, victim, TO_VICT);
	    return;
	}

	else if (!can_get_gems(victim, amount) )
	{
	    act("$N cannot carry that amount.", ch, NULL, victim, TO_CHAR);
	    act("$n tries to give you some gems, but they are too heavy.", ch, NULL, victim, TO_VICT);
	    return;
	}

	if (!has_got_money(ch, arg2, amount) ) return;

	if (!str_prefix(arg2, "copper") )
	{
	    ch->copper          -= amount;
	    victim->copper      += amount;
	    if (amount == 1) sprintf(denom, "copper piece");	    
	    else sprintf(denom, "copper pieces");
	}
	else if (!str_prefix(arg2, "silver"))
	{
	    ch->silver          -= amount;
	    victim->silver      += amount;
	    if (amount == 1) sprintf(denom, "silver coin");	    
	    else sprintf(denom, "silver coins");
	}
	else if (!str_prefix(arg2, "gold"))
	{
	    ch->gold            -= amount;
	    victim->gold        += amount;
	    if (amount == 1) sprintf(denom, "gold coin");	    
	    else sprintf(denom, "gold coins");
	    if (IS_SET(victim->act, ACT_IS_HEALER) && amount >= 200
		&& IS_NPC(victim))
	    {
		if (!TIME_UP(ch, TIMER_CAN_USE_HEALER))
		{
		    do_say(victim, "Wait a bit while I rememorise spells.");
		    return;
		}
		SET_TIMER(ch, TIMER_CAN_USE_HEALER, dice(2, 6) );
		act("$N lays $S hands upon $n.", ch, NULL, victim, TO_NOTVICT);
		act("$N lays $S hands upon you.", ch, NULL, victim, TO_CHAR);
		stc("You feel totally restored.\n\r", ch);
	 	act("$n shimmers with a golden aura.", ch, NULL, NULL, TO_ROOM);
		restore_char(ch);
		victim->gold = 0;
	    }
	}
	else if (!str_prefix(arg2, "emeralds"))
	{
	    ch->emeralds        -= amount;
	    victim->emeralds    += amount;
	    if (amount == 1) sprintf(denom, "emerald");	    
	    else sprintf(denom, "emeralds");
	}
	else if (!str_prefix(arg2, "sapphires"))
	{
	    ch->sapphires       -= amount;
	    victim->sapphires   += amount;
	    if (amount == 1) sprintf(denom, "sapphire");	    
	    else sprintf(denom, "sapphires");
	}
	else if (!str_prefix(arg2, "diamonds"))
	{
	    ch->diamonds        -= amount;
	    victim->diamonds    += amount;
	    if (amount == 1) sprintf(denom, "diamond");	    
	    else sprintf(denom, "diamonds");
	}
	sprintf(buf, "You give $N %d %s.", amount, denom);
	act(buf, ch, NULL, victim, TO_CHAR);
	sprintf(buf, "$n gives you %d %s.", amount, denom);
	act(buf, ch, NULL, victim, TO_VICT);
	sprintf(buf, "$n gives $N %d %s.", amount, denom);
	act(buf, ch, NULL, victim, TO_NOTVICT);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (victim->cur_form == FRM_MIST
	|| victim->cur_form == FRM_CRIMSON)
    {
	send_to_char("But they are insubstantial.\n\r", ch);
	return;
    }

    if (IS_OBJ_STAT2(obj, ITEM_DESIRED) )
    {
	act("But you want to keep $p.", ch, obj, NULL, TO_CHAR);
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent,skill;

    /* find out what */
    if (argument == '\0')
    {
	send_to_char("Envenom what item?\n\r",ch);
	return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    skill = get_skill(ch, gsn_envenom) + 10;

    if (obj== NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
	if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
	    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (number_percent() < skill)  /* success! */
	{
	    act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
	    act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
	    if (!obj->value[3])
	    {
		obj->value[3] = 1;
		check_improve(ch,gsn_envenom,TRUE,4);
	    }
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}

	act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	if (!obj->value[3])
	    check_improve(ch,gsn_envenom,FALSE,666);
	WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
	if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
	    act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (obj->value[3] < 0 
	||  attack_table[obj->value[3]].damage == DAM_BASH)
	{
	    send_to_char("You can only envenom edged weapons.\n\r",ch);
	    return;
	}

	if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	{
	    act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	percent = number_percent();
	if (percent < skill)
	{
 
	    af.where     = TO_WEAPON;
	    af.type      = gsn_poison;
	    af.level     = ch->level * percent / 100;
	    af.duration  = ch->level/2 * percent / 100;
	    af.location  = 0;
	    af.modifier  = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);
 
	    act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
	    act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,TRUE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}
	else
	{
	    act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,FALSE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}
    }
 
    act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;
    bool river = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (ch->in_room->sector_type == SECT_WATER_SWIM ||
       ch->in_room->sector_type == SECT_WATER_NOSWIM ||
       ch->in_room->sector_type == SECT_UNDERWATER)
    {
	found = TRUE;
	river = TRUE;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    if (river)
    {
    sprintf(buf,"You fill $p with water.");
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with water.");
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = LIQ_WATER;
    obj->value[1] = obj->value[0];
    return;
    }

    sprintf(buf,"You fill $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("That's not a drink container.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR);
	
	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM);
	return;
    }

    if ((in = get_obj_here(ch,argument)) == NULL)
    {
	vch = get_char_room(ch,argument);

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_obj_by_type(vch,ITEM_DRINK_CON);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into other drink containers.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,out,in,TO_CHAR);
	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
	sprintf(buf,"You pour some %s for $N.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_CHAR);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT);
	sprintf(buf,"$n pours some %s for $N.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_NOTVICT);
	
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;
    AFFECT_DATA af;
    int drinklim;

    drinklim = get_size(ch) * 20;
    if (drinklim < 60) drinklim = 60;

    if (ch->secondaryrace == RACE_DRAGON) drinklim *= 2;

    if (ch->fighting != NULL)
    {
	stc("Not while fighting!\n\r", ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN
		|| obj->item_type == ITEM_RAINBOW_POOL )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 20 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;
 
    case ITEM_CAULDRON:

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char( "The liquid doesn't seem to want to leave the container.\n\r", ch);
	return;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("It is empty.\n\r", ch);
	return;
    }
    else 
    {
	act( "$n drinks out of $p.", ch, obj, NULL, TO_ROOM );
	act( "You drink out of $p.", ch, obj, NULL ,TO_CHAR );
	if (obj->value[2] != 0) obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	if (obj->value[3] != 0) obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	if (obj->value[4] != 0) obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
	obj->value[1] -= (get_size(ch) + 1);
	if (obj->value[1] < 0) obj->value[1] = 0;        
	return;
    }
		
    case ITEM_POTION:
    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char( "The liquid doesn't seem to want to leave the container.\n\r", ch);
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );
    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
    extract_obj( obj );
    return;


    case ITEM_FOUNTAIN:
	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}
	amount = liq_table[liquid].liq_affect[4] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	amount = liq_table[liquid].liq_affect[4];
	amount = UMIN(amount, obj->value[1]);
	break;

    case ITEM_RAINBOW_POOL:
	act( "$n drinks out of $p.", ch, obj, NULL, TO_ROOM );
	act( "You drink out of $p.", ch, obj, NULL ,TO_CHAR );
	restore_char(ch);
	extract_obj(obj);
	return;
	break;

    }
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) 
    &&  ch->pcdata->condition[COND_FULL] > drinklim)
    {
	send_to_char("You're too full to drink more.\n\r",ch);
	return;
    }

    act( "$n drinks $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_ROOM );
    act( "You drink $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_CHAR );

    if (!str_cmp("blood",liq_table[liquid].liq_name)
	&& ch->secondaryrace == 3)
    {
	send_to_char("You take a small sip - this is dead blood, you gag and vomit.\n\r", ch);
	act("$n gags and vomit from drinking dead blood.", ch, NULL, NULL, TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level     = dice (10, 10); 
	af.duration  = dice (1, 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
	return;
    }

    if (ch->secondaryrace == 3)
    {
	send_to_char("You take a small sip, and immediately choke.\n\r",ch);
	act("$n takes a small sip, and immediately chokes.", ch, NULL,NULL, TO_ROOM);
	return;
    }

    gain_condition( ch, COND_DRUNK,
	amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    gain_condition( ch, COND_FULL,
	amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
    gain_condition( ch, COND_THIRST,
	amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
    gain_condition(ch, COND_HUNGER,
	amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "You feel drunk.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > drinklim-1 )
	send_to_char( "You are full.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > drinklim-1)
	send_to_char( "Your thirst is quenched.\n\r", ch );
	
    if ( obj->value[3] > 0 )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You choke and gag.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level         = number_fuzzy(amount); 
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
    }
	
    if (obj->value[0] > 0)
	obj->value[1] -= amount;

    return;
}

void check_grow_arms(CHAR_DATA *ch)
{
    if (!TIME_UP(ch, TIMER_CAN_EAT_ARMS) )
    {
	send_to_char("Your stomach feels a bit queasy for a second, then it passes.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_THIRD_ARM_GROWING) && !TIME_UP(ch, TIMER_FOURTH_ARM_GROWING) )
    {
	send_to_char("You feel immense pain in your stomach.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_THIRD_ARM_GROWING) && BODY_PART(ch, BODY_ENABLE_FOURTH_ARM) )
    {
	send_to_char("You feel immense pain in your stomach.\n\r", ch);
	return;
    }

    if (!TIME_UP(ch, TIMER_FOURTH_ARM_GROWING) && BODY_PART(ch, BODY_ENABLE_THIRD_ARM) )
    {
	send_to_char("You feel immense pain in your stomach.\n\r", ch);
	return;
    }

    if (BODY_PART(ch, BODY_ENABLE_THIRD_ARM) && BODY_PART(ch, BODY_ENABLE_FOURTH_ARM) )
    {
	send_to_char("You cannot grow any more arms.\n\r", ch);
	return;
    }

    SET_TIMER(ch, TIMER_CAN_EAT_ARMS, dice(2, 8) );

    if (!BODY_PART(ch, BODY_ENABLE_THIRD_ARM) && (BODY_PART(ch, BODY_ENABLE_FOURTH_ARM)
	|| !TIME_UP(ch, TIMER_FOURTH_ARM_GROWING) ) )
    {
	send_to_char("Something starts go grow in your stomach.\n\r", ch);
	SET_TIMER(ch, TIMER_THIRD_ARM_GROWING, dice(3, 6) );
	return;
    }

    if (!BODY_PART(ch, BODY_ENABLE_FOURTH_ARM) && (BODY_PART(ch, BODY_ENABLE_THIRD_ARM)
	|| !TIME_UP(ch, TIMER_THIRD_ARM_GROWING) ) )
    {
	send_to_char("Something starts go grow in your stomach.\n\r", ch);
	SET_TIMER(ch, TIMER_FOURTH_ARM_GROWING, dice(3, 6) );
	return;
    }

    if (!BODY_PART(ch, BODY_ENABLE_THIRD_ARM) && !BODY_PART(ch, BODY_ENABLE_FOURTH_ARM) )
    {
	send_to_char("Something starts go grow in your stomach.\n\r", ch);
	SET_TIMER(ch, TIMER_THIRD_ARM_GROWING, dice(3, 6) );
	return;
    }

}


void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int eatlim;
    bool caneat = FALSE;

    eatlim = 60;

    if (ch->fighting != NULL)
    {
	stc("Not while fighting!\n\r", ch);
	return;
    }

    if (get_size(ch) == SIZE_LARGE)
	eatlim += 20;

    if (get_size(ch) == SIZE_HUGE)
	eatlim += 30;

    if (get_size(ch) == SIZE_GIANT)
	eatlim += 50;

    if (ch->secondaryrace == RACE_DRAGON)
	eatlim *= ch->generation/3;
   
    if ( ch->secondaryrace == 3 && ch->level < 113)
    {
	send_to_char("You do not feel inclined to eat anything.\n\r",ch);
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_PILL)
	caneat = TRUE;

    if (obj->item_type == ITEM_BODY_PART && ch->secondaryrace == RACE_DAEMON)
	caneat = TRUE;

    if ( (obj->item_type == ITEM_BODY_PART || obj->item_type == ITEM_CORPSE_PC
	    || obj->item_type == ITEM_CORPSE_NPC) && (ch->secondaryrace == RACE_DRAGON
	    || ch->secondaryrace == RACE_LUPINE) )
	caneat = TRUE;

    if (IS_IMMORTAL(ch)) caneat = TRUE;

    if (!IS_IMMORTAL(ch) )
    {
	if (!caneat)
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}
/*	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > eatlim)
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}*/
    }

    if (ch->secondaryrace == RACE_DRAGON
	|| ch->secondaryrace == RACE_LUPINE
	|| ch->secondaryrace == RACE_DAEMON)
    {
        act( "$n devours $p.",  ch, obj, NULL, TO_ROOM );
        act( "You devour $p.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
        act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
        act( "You eat $p.", ch, obj, NULL, TO_CHAR );
    }

    switch ( obj->item_type )
    {

    case ITEM_BODY_PART:
	if (!IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_HUNGER];
	    gain_condition( ch, COND_FULL, 10 );
	    gain_condition( ch, COND_HUNGER, 10);
	    if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 20 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > eatlim-1 )
		send_to_char( "You are full.\n\r", ch );
	    if (ch->secondaryrace == RACE_DAEMON && ch->generation > 3
		&& !str_prefix("the arm of", obj->short_descr)
		&& ch->power[DISC_DAEM_ATTA] > 8 )
		check_grow_arms(ch);
	}
	break;

    case ITEM_CORPSE_PC:
    case ITEM_CORPSE_NPC:
	if (!IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_HUNGER];
	    gain_condition( ch, COND_FULL, 60 );
	    gain_condition( ch, COND_HUNGER, 60);
	    if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 20 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > eatlim-1 )
		send_to_char( "You are full.\n\r", ch );
	}
	break;


    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_HUNGER];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    gain_condition( ch, COND_HUNGER, obj->value[1]);
	    if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 20 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > eatlim-1 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.where     = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level     = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}

	    /* MUUUUUUUUUUUUUUUUUUUUSHROOOOOOM! */
	if ( obj->value[2] == MUSHROOM_SPEED)
	{
	    ADD_TIMER(ch, TIMER_ON_SPEED, dice(3, 6));
	    send_to_char("Woh!  Everything is going soooo slowly.\n\r",ch);
	}
	else if (obj->value[2] == MUSHROOM_LSD)
	{
	    ADD_TIMER(ch, TIMER_ON_LSD, dice(2, 8));
	    send_to_char("Nice colours man!\n\r",ch);
	}
	else if (obj->value[2] == MUSHROOM_HASH)
	{
	    ADD_TIMER(ch, TIMER_ON_HASH, dice(4, 3));
	    send_to_char("The world starts to spin.\n\r",ch);
	}
	else if (obj->value[2] == MUSHROOM_KING)
	{
	    CHAR_DATA *mob;

	    room_message(ch->in_room, "The mushroom screams 'OI!' and leaps to the ground.  And grows.");
	    mob = create_mobile( get_mob_index( MOB_VNUM_MUSHROOM_KING ) );
	    char_to_room(mob, ch->in_room, ch->altitude);
	    set_fighting(ch, mob);
	}
	else if (obj->value[2] == MUSHROOM_WIPE_SPELLS
		&& !IS_NPC(ch))
	{
	    int i, j;

	    if (IS_NPC(ch)) return;

	    for (i = 1; i < 11; i++)
		for (j = 0; j < 10; j++)
		    ch->pcdata->memorize[i][j] = -1;

	    send_to_char("Your mind feels fuzzy.  Whats your name again?\n\r",ch);
	}
	else if (obj->value[2] == MUSHROOM_GET_ALL_SPELLS
		&& !IS_NPC(ch))
	{
	    int i, j;

	    if (IS_NPC(ch)) return;

	    for (i = 1; i < 11; i++)
		for (j = 0; j < 10; j++)
		    ch->pcdata->memorize[i][j] = ch->pcdata->last_memorized[i][j];

	    send_to_char("You feel mentally invigorated.\n\r",ch);
	    do_say(ch, "I feel mentally invigorated.");
	}
	else if (obj->value[2] == MUSHROOM_DEATH)
	{
	    send_to_char("Euck.  Deadly poison.  You're dea...\n\r",ch);
	    destroy_char(ch, ch, TRUE);
	}
	else if (obj->value[2] == MUSHROOM_PASSOUT)
	{
	    send_to_char("Euck.  Deadly poison.  You're dea...\n\r",ch);
	    ADD_TIMER(ch, TIMER_UNCONCIOUS, dice(1, 2));
	    ch->position = POS_SLEEPING;
	}
	else if (obj->value[2] == MUSHROOM_WALL)
	{
	    int door;
	    send_to_char("How odd.  Nothing happened.\n\r",ch);
	    for (door = 0; door < 6; door++)
		make_wall(ch->in_room, door, EX_MUSHROOM_WALL);
	}
	else if (obj->value[2] == MUSHROOM_ARM_GROW
		&& !IS_NPC(ch))
	{
	    send_to_char("You feel a pain in your stomach.\n\r",ch);
	    ADD_TIMER(ch, TIMER_THIRD_ARM_GROWING, dice(2, 4) );
	}
	else if (obj->value[2] > 0)
	{
	    send_to_char("Damn nicest mushroom you've ever tasted.\n\r",ch);
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;

    default: if (ch->secondaryrace == RACE_DRAGON && ch->level < 113)
	     {
		gain_condition(ch, COND_FULL, 20);
		gain_condition(ch, COND_HUNGER, 10);
	     }
    }

    if (obj != NULL) extract_obj( obj );
    return;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( obj->artifact == RELIC_WIND_BOOTS
	  && !is_default_flying(ch) )
    {
	send_to_char("You float to the ground.\n\r", ch);
	act("$n floats to the ground.", ch, NULL, NULL, TO_ROOM);
    }

    if ( obj->artifact == ARTIFACT_SYMAL)
    {
	send_to_char("The jewel symal pulsates slowly, but stays put.\n\r", ch);
	return FALSE;
    }

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    unequip_char( ch, obj );
    return TRUE;
}


/* Free hand counter */
bool sremove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}

/* Free hand counter */

int free_hand(CHAR_DATA *ch)
{
    int fh=2;

    if (BODY_PART(ch,BODY_LEFT_ARM)) fh--;
    else if (BODY_PART(ch, BODY_LEFT_HAND)) fh--;
    if (BODY_PART(ch,BODY_RIGHT_ARM)) fh--;
    else if (BODY_PART(ch, BODY_RIGHT_HAND)) fh--;
    if (BODY_PART(ch,BODY_THIRD_ARM)) fh++;
    if (BODY_PART(ch,BODY_FOURTH_ARM)) fh++;
    if (get_eq_char( ch, WEAR_LEFT_HAND  ) != NULL) fh--;
    if (get_eq_char( ch, WEAR_RIGHT_HAND ) != NULL) fh--;
    if (get_eq_char( ch, WEAR_THIRD_HAND ) != NULL) fh--;
    if (get_eq_char( ch, WEAR_FOURTH_HAND) != NULL) fh--;

    return fh;
}

int free_hold_slot(CHAR_DATA *ch)
{
    if (!BODY_PART(ch,BODY_RIGHT_ARM) && !BODY_PART(ch, BODY_RIGHT_HAND))
	if (get_eq_char(ch, WEAR_RIGHT_HAND) == NULL) return WEAR_RIGHT_HAND;
    if (!BODY_PART(ch,BODY_LEFT_ARM) && !BODY_PART(ch, BODY_LEFT_HAND))
	if (get_eq_char(ch, WEAR_LEFT_HAND) == NULL) return WEAR_LEFT_HAND;
    if (BODY_PART(ch,BODY_THIRD_ARM) && BODY_PART(ch,BODY_THIRD_HAND))
	if (get_eq_char(ch, WEAR_THIRD_HAND) == NULL) return WEAR_THIRD_HAND;
    if (BODY_PART(ch,BODY_FOURTH_ARM) && BODY_PART(ch,BODY_FOURTH_HAND))
	if (get_eq_char(ch, WEAR_FOURTH_HAND) == NULL) return WEAR_FOURTH_HAND;
     return WEAR_NONE;
}    

/*
 * Limb loss routine
 * Should be FUN!  heh heh heh
 */

OBJ_DATA * wither_arm(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    int arm;

    arm = dice(1,2);

    if (BODY_PART(ch, BODY_LEFT_ARM)) arm = 2;
    if (arm == 2 && BODY_PART(ch, BODY_RIGHT_ARM))
	return NULL;

    if (arm == 2) obj = get_eq_char(ch, WEAR_LEFT_HAND);
    else obj = get_eq_char(ch, WEAR_RIGHT_HAND);

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room, ch->altitude );
	SET_BIT(ch->body_form, BODY_LEFT_ARM);
	return obj;
   
}

void display_clatter( CHAR_DATA *ch, OBJ_DATA *obj)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%s's %s drops to the floor.",GET_NAME(ch), obj->short_descr);
    act(buf, ch, NULL, NULL, TO_ROOM);
    act("Your $p drops to the floor.",ch, NULL, obj, TO_CHAR);
}

void recalc_wear(CHAR_DATA *ch)
{
    OBJ_DATA *obj, *obj_next;
    for (obj = ch->carrying ; obj != NULL ; obj = obj_next )
    {
	obj_next = obj->next_content;

	if (obj->wear_loc == WEAR_NONE) continue;	
	if (!IS_SET(obj->wear_flags, ITEM_TAKE)) continue;
	if (IS_OBJ_STAT3(obj, ITEM_STRETCHY)) continue;

	if (obj->item_type == ITEM_ARMOR)
	{
	    if (obj->size < get_size(ch) && 
	        !(obj->size == SIZE_SMALL && get_size(ch) == SIZE_MEDIUM) )
	    {
	        act("You burst out of $p.", ch, obj, NULL, TO_CHAR);
	        act("$n bursts out of $p.", ch, obj, NULL, TO_ROOM);
	        if (!damage_obj(obj, get_size(ch) * 10))
	        {
	   	    act("$p falls to the floor.", ch, obj, NULL, TO_ALL);
		    bug ("obj_from_char here", 0);
		    obj_from_char(obj);
		    obj_to_room(obj, ch->in_room, ch->altitude);
		    continue;
		}
		else continue;
	    }
	    else if (obj->size > get_size(ch) && 
	        !(obj->size == SIZE_MEDIUM && get_size(ch) == SIZE_SMALL) )
	    {
	        act("$n is too small for $p, so it drops to the floor.", ch, obj, NULL, TO_ROOM);
	        act("You are too small for $p, so it drops to the floor.", ch, obj, NULL, TO_CHAR);
	        obj_from_char(obj);
	        obj_to_room(obj, ch->in_room, ch->altitude);
	        continue;
	    }
	    else continue;
	}
	if (is_wrong_size(ch, obj))
	{
	    if (obj->carried_by == ch)  /* Still in hand? */
	    {
	        obj_from_char(obj);
	        obj_to_room(obj, ch->in_room, ch->altitude);
	        act("$p falls to the floor.", ch, obj, NULL, TO_CHAR);	
	        act("$p, carried by $n falls to the floor.", ch, obj, NULL, TO_ROOM);
	    }
	    continue;
	}
	/* end of if statement */
    } 
}

bool is_wrong_size(CHAR_DATA *ch, OBJ_DATA *obj)
{
    int hands_for_item = calc_hands_needed(ch, obj);
    if (obj->wear_loc == WEAR_NONE) hands_for_item = 0;

    if (obj->item_type == ITEM_ARMOR)
    {
        if (get_size(ch) == obj->size) return FALSE;
        if (get_size(ch) == SIZE_MEDIUM && obj->size == SIZE_SMALL) return FALSE;
        if (get_size(ch) == SIZE_SMALL && obj->size == SIZE_MEDIUM) return FALSE;

        if (obj->size > get_size(ch))
        {
            act("$p is much too big for you to wear.", ch, obj, NULL, TO_CHAR);
            return TRUE;
        }

        if (obj->size < get_size(ch))
        { 
            act("$p is much too small for you to wear.", ch, obj, NULL, TO_CHAR);
            return TRUE;
        }

        return FALSE;
    }

    /* No dragons using darts, wands etc... */
    if (obj->size <= get_size(ch) - 2)
    {
	act("$p slips through your fingers.", ch, obj, NULL, TO_CHAR);
	act("$n tries to hold $p, but it slips through $s fingers.", ch, obj, NULL, TO_ROOM);
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room, ch->altitude);
	return TRUE;
    }

    /* No arms or hands */
    if (hand_slots(ch) == 0)
    {
	stc("You cannot hold anything!\n\r", ch);
	return TRUE;
    }

    /* Way way too big things */
    if (calc_hands_needed(ch, obj) == 100)
    {
	act("$p is way too big and heavy to hold.", ch, obj, NULL, TO_CHAR);
	return TRUE;
    }

    /* No arms left */
    if (calc_hands_needed(ch, obj) == 1
	&& hand_slots(ch) - (hands_taken_up(ch) - hands_for_item) < 1)
    {
	act("You need a free hand to hold $p.", ch, obj, NULL, TO_CHAR);
	return TRUE;
    } 

    /* Not enough hands for a 2 handed weapon */
    if (calc_hands_needed(ch, obj) == 2
	&& hand_slots(ch) - (hands_taken_up(ch) - hands_for_item) < 2)
    {
	act("You need two hands free to hold $p.", ch, obj, NULL, TO_CHAR);
	return TRUE;
    } 

    /* Not enough hands for a 2 handed weapon */
    if (calc_hands_needed(ch, obj) == 4
	&& hand_slots(ch) - (hands_taken_up(ch) - hands_for_item) < 4)
    {
	act("You would need at least four arms to hold $p.", ch, obj, NULL, TO_CHAR);
	return TRUE;
    } 
  
    return FALSE;
}

int calc_hands_needed(CHAR_DATA *ch, OBJ_DATA *obj)
{
    crash("Calc hands needed for an obj function");

    if (obj == NULL || ch == NULL) return 0;
    if (obj->item_type == ITEM_ARMOR) return 0;

    if (obj->item_type == ITEM_BOW)
    {	
	if (obj->size >= get_size(ch) + 2) return 100;
	return 2;
    }

    if (get_size(ch) >= obj->size) return 1;
    if (get_size(ch) == obj->size - 1) return 2;
    if (get_size(ch) == obj->size - 2) return 4;
    return 100;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool show )
{
    if (obj->artifact == RELIC_WIND_BOOTS
	&& !is_flying(ch) )
    {
	send_to_char("You rise into the air.\n\r", ch);
	act("$n rises up into the air.", ch, NULL, NULL, TO_ROOM);
    }

    /* Warlocks can only use daggers! */
    if (ch->secondaryrace == RACE_WARLOCK
	&& obj->item_type == ITEM_WEAPON
	&& obj->value[0] != WEAPON_DAGGER)
    {
	stc("Warlocks may only wield a dagger in combat.\n\r", ch);
	return;
    }

    if (is_wrong_size(ch, obj)) return;

    if ( obj->item_type == ITEM_LIGHT )
    {
	if (!free_hand(ch))
	{
	    if (show) send_to_char("Your hands are full.\n\r",ch);
	    return;
	}
	act( "$n lights $p.", ch, obj, NULL, TO_ROOM );
	act( "You light $p",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, free_hold_slot(ch) );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( where_name[WEAR_FINGER_L][2],    ch, obj, NULL, TO_ROOM );
	    act( where_name[WEAR_FINGER_L][1],  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( where_name[WEAR_FINGER_L][2],   ch, obj, NULL, TO_ROOM );
	    act( where_name[WEAR_FINGER_R][1], ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( where_name[WEAR_NECK_1][2],   ch, obj, NULL, TO_ROOM );
	    act( where_name[WEAR_NECK_1][1], ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( where_name[WEAR_NECK_2][2],   ch, obj, NULL, TO_ROOM );
	    act( where_name[WEAR_NECK_2][1], ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( where_name[WEAR_BODY][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_BODY][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if (IS_SET(ch->flag4, AFF_HORN))
	{
	    send_to_char("You can't wear a helmet while you have horns.\n\r", ch);
	    return;
	}

	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( where_name[WEAR_HEAD][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_HEAD][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if (IS_SET(ch->flag2, AFF_TAIL))
	{
	    send_to_char("You can't wear anything on your legs, because your tail is too large.\n\r", ch);
	    return;
	}

	if (lost_one_leg(ch) || lost_both_legs(ch) )
	{
	    stc("You need two legs to wear this item.\n\r", ch);
	    return;
	}

	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( where_name[WEAR_LEGS][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_LEGS][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if (lost_one_leg(ch) || lost_both_legs(ch) )
	{
	    stc("You need two legs to wear this item.\n\r", ch);
	    return;
	}

	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( where_name[WEAR_FEET][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_FEET][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( where_name[WEAR_HANDS][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_HANDS][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( where_name[WEAR_ARMS][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_ARMS][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( where_name[WEAR_ABOUT][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_ABOUT][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( where_name[WEAR_WAIST][2],   ch, obj, NULL, TO_ROOM );
	act( where_name[WEAR_WAIST][1], ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( where_name[WEAR_WRIST_L][2],
		ch, obj, NULL, TO_ROOM );
	    act( where_name[WEAR_WRIST_L][1],
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( where_name[WEAR_WRIST_R][2],
		ch, obj, NULL, TO_ROOM );
	    act( where_name[WEAR_WRIST_L][1],
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	if (IS_SET(ch->flag2, AFF_CLAW))
	{
	    send_to_char("You can't wear a shield with your claws extended.\n\r", ch);
	    return;
	}

	if (!free_hand(ch))
	{
	    if (show) send_to_char("Your hands are full.\n\r",ch);
	    return;
	}

	act( "$n uses $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You use $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, free_hold_slot(ch) );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill,iWear;

	if (IS_SET(ch->flag2, AFF_CLAW))
	{
	    send_to_char("You can't wield a weapon with your claws extended.\n\r", ch);
	    return;
	}

	if (!free_hand(ch))
	{
	    if (show) send_to_char("Your hands are full.\n\r",ch);
	    return;

	}

	iWear = free_hold_slot(ch);

		if ( !IS_NPC(ch) 
		&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield  
			* 10))
		{
		    send_to_char( "It is too heavy for you to wield.\n\r", ch );
		    return;
		}

		act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
		act( "You wield $p.", ch, obj, NULL, TO_CHAR );
		equip_char( ch, obj, iWear );

		sn = get_weapon_snsl(ch, iWear);

		if (sn == gsn_hand_to_hand)
		   return;

		skill = get_weapon_skill(ch,sn);
 
		if (skill >= 100)
		    act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
		else if (skill > 85)
		    act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
		else if (skill > 70)
		    act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
		else if (skill > 50)
		    act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
		else if (skill > 25)
		    act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
		else if (skill > 1)
		    act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
		else
		    act("You don't even know which end is up on $p.",
			ch,obj,NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if (IS_SET(ch->flag2, AFF_CLAW))
	{
	    send_to_char("You can't hold an item with your claws extended.\n\r", ch);
	    return;
	}

	if (!free_hand(ch))
	{
	    if (show) send_to_char("Your hands are full.\n\r",ch);
	    return;
	}
	act( "$n holds $p.", ch, obj, NULL, TO_ROOM );
	act( "You hold $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, free_hold_slot(ch) );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
	    return;
	act(where_name[WEAR_FLOAT][2],ch,obj,NULL,TO_ROOM);
	act(where_name[WEAR_FLOAT][1],ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if (!form_data[ch->cur_form].can_wear && ch->cur_form != 0)
    {
	send_to_char("You can't wear items in this form.\n\r", ch);
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	wear_obj( ch, obj, TRUE, TRUE );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	OBJ_DATA *obj_next;
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
		remove_obj(ch,obj->wear_loc, TRUE);
	}
	return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}

void do_sremove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if (!str_cmp(arg, "all"))
    {
	OBJ_DATA *obj_next;
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
		sremove_obj(ch,obj->wear_loc, TRUE);
	}
	return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    sremove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *obj2 = NULL;
    int silver;
    
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to the gods, who graciously decline.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "The powers appreciate your offer and may accept it later.\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if (obj->sw_char != NULL
	&& !IS_SET(obj->extra_flags3, ITEM_DESTROY_CHAR_IF_SACCED))
    {
	stc("You cannot destroy the item.\n\r", ch);
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
	{
	   send_to_char(
	     "The powers frown at your actions.\n\r",ch);
	   return;
	}
    }

    if (IS_OBJ_STAT2(obj, ITEM_TELEPORT_PROTECTION))
    {
	act("$p vanishes in a puff of smoke.", ch, obj, NULL, TO_ALL);
	obj_from_room(obj);
	obj_to_room(obj, get_rand_room(), 0);
	return;
    }	

    if (IS_OBJ_STAT2(obj, ITEM_INDESTRUCTABLE))
    {
	stc("You cannot destroy the artifact.\n\r", ch);
	return;
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }
		
    silver = 0;

    sprintf(buf,"%s ignites and is consumed in flames.\n\r",obj->short_descr);
    buf[0] = UPPER(buf[0]);
    send_to_char(buf,ch);
    ch->hit += 1;
    
    if (IS_SET(ch->act,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
	{
	    if (gch->altitude != ch->altitude) continue;
	    if ( is_same_group( gch, ch ) )
	    members++;
	}

	if ( members > 1 && silver > 1)
	{
	    sprintf(buffer,"%d",silver);
	    do_split(ch,buffer);        
	}
    }

    act( "$n sacrifices $p to the powers.", ch, obj, NULL, TO_ROOM );
    wiznet("$N sends up $p as a burnt offering.",
	   ch,obj,WIZ_SACCING,0,0);
    if (!IS_NPC(ch)
	&& obj->item_type == ITEM_CORPSE_PC
	&& ch->pcdata->voodoo_points > 10)
    {
	act("$p crumbles into fine dust.", ch, obj, NULL, TO_ROOM);
	obj2 = create_object(get_obj_index(12010), 0);
        SET_BIT(obj2->extra_flags2, ITEM_VOO_DEAD);

        sprintf(buf, obj2->short_descr,obj->voodoo_name );
        free_string(obj2->short_descr);
        obj2->short_descr = str_dup(buf);

        sprintf(buf, obj2->name, obj->voodoo_name );
        free_string(obj2->name);
        obj2->name = str_dup(buf);

        free_string(obj2->voodoo_name);
        obj2->voodoo_name = str_dup(obj->voodoo_name);

	obj_to_room(obj2, ch->in_room, ch->altitude);
	obj2->timer = ch->pcdata->voodoo_points / 2;
    }

    /* Check for a quest item thingy... ho ho ho */

    if (!IS_NPC(ch) && IS_FLAG4(ch, AFF_ON_A_QUEST)
	&& IS_ROOM_STAT2(ch->in_room, ROOM_QUEST_CENTRE) )
    {
	if (ch->pcdata->quest_item_1 != NULL
	    && !str_cmp(ch->pcdata->quest_item_1, obj->short_descr) )
	{
	    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ITEM_1);
	    ch->pcdata->quest_item_1 = NULL;
	    send_to_char("The flames form green smoke rings which drift skyward.\n\r", ch);
	    act("Small smoke rings drift off from $p.", ch, obj, NULL, TO_ROOM);
	}
	else if (ch->pcdata->quest_item_2 != NULL
	    && !str_cmp(ch->pcdata->quest_item_2, obj->short_descr) )
	{
	    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ITEM_2);
	    ch->pcdata->quest_item_2 = NULL;
	    send_to_char("The flames form small orange smoke rings which drift skyward.\n\r", ch);
	    act("Small smoke rings drift off from $p.", ch, obj, NULL, TO_ROOM);
	}
	else if (ch->pcdata->quest_item_3 != NULL
	    && !str_cmp(ch->pcdata->quest_item_3, obj->short_descr) )
	{
	    REMOVE_BIT(ch->pcdata->quest_status, QUEST_ITEM_3);
	    ch->pcdata->quest_item_3 = NULL;
	    send_to_char("The flames form pink smoke rings which drift skyward.\n\r", ch);
	    act("Small smoke rings drift off from $p.", ch, obj, NULL, TO_ROOM);
	}
    }

    extract_obj( obj );
    return;
}

void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if (obj != NULL && obj->item_type == ITEM_CAULDRON && obj->value[1] == 0)
    {
	send_to_char("It is empty.\n\r", ch);
	return;
    }
    
    else if (obj != NULL && obj->item_type == ITEM_CAULDRON)
    {
	act( "$n drinks out of $p.", ch, obj, NULL, TO_ROOM );
	act( "You drink out of $p.", ch, obj, NULL ,TO_CHAR );
	if (obj->value[2] != 0) obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	if (obj->value[3] != 0) obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	if (obj->value[4] != 0) obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
	obj->value[1] -= (get_size(ch) + 1);
	if (obj->value[1] < 0) obj->value[1] = 0;        
	return;
    }


    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION
	 && obj->item_type != ITEM_SPEC_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char( "The liquid doesn't seem to want to leave the container.\n\r", ch);
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    if (obj->item_type == ITEM_SPEC_POTION)

	/* value 0 = level    value 1 = type of potion (bigger the
	   better, results currently range from 1 to 10
	   but maybe more will appear in the future 

	   30 and above are my magical potions !!! eg brewing */

    {	
	/* Gaining levels at brewing */

	if (obj->value[1] == POTION_GAIN_BREW
		&& ch->power[DISC_WARL_BREW] < 10)
	{
	    ch->power[DISC_WARL_BREW]++;
	    stc("You feel more experienced.  *HIC*\n\r", ch);
	    extract_obj(obj);
	    return;
 	}

	if (obj->value[1] == POTION_BEER)
	{
	    stc("The potion contains strong beer.  *HIC*\n\r", ch);
	    gain_condition( ch, COND_DRUNK, 3);
	    extract_obj(obj);
	    return;
 	}

	switch (obj->value[1])
	{
	    case 1:
	        rand_to_char(ch, "The potion tastes salty, you spit it out.\n\r",
		  "The potion tastes of strawberries and cream.\n\r",
		  "You feel disorientated for a second.  Maybe that potion was poisoned.\n\r",
		  "Wispy vapours of blue gas stream out of the bottle, and then vanish.\n\r", 
		  "The potion contains strong beer.  *HIC*\n\r");
	        break;
	}

        extract_obj( obj );
	return;
    }


    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }
 
   if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char( "You feel momentarily confused, you cannot understand the mystical symbols.\n\r", ch);
	return;
    }
 

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls))
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }

    else
    {
	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[4], scroll->value[0], ch, victim, obj );
	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_obj_by_type( ch, ITEM_STAFF ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char( "The staff fizzles and sparks, but nothing happens.\n\r", ch);
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level 
	||   number_percent() >= 20 + get_skill(ch,gsn_staves))
	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next    = vch->next_in_room;
	    if (vch->altitude != ch->altitude) continue;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_obj_by_type( ch, ITEM_WAND ) ) == NULL )
    {
	if ( (wand = get_obj_by_type( ch, ITEM_FEATHER ) ) == NULL )
	{
	    send_to_char( "You hold nothing in your hand.\n\r", ch );
	    return;
	}
    }

    if ( wand->item_type != ITEM_WAND
	&& wand->item_type != ITEM_FEATHER )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	send_to_char( "The wand fizzles and sparks, but nothing happens.\n\r", ch);
	return;
    }

    if (wand->item_type == ITEM_FEATHER)
    {
	/* put the weird feather shit in here - 4 types of them */
	if (wand->value[0] == FEATHER_COPPER)
	{
	    act("$p explodes into smoke, you inhale and feel refreshed.", ch, wand, NULL, TO_CHAR);
	    ch->move = UMIN(ch->max_move, ch->move * 3 / 2);
	    act("$n holds $p in the air and it explodes into smoke.", ch, wand, NULL, TO_ROOM);
	    if (dice(1, 6) > 2) extract_obj(wand);
	        else wand->value[0]++;
	    return;
	}
	else if (wand->value[0] == FEATHER_SILVER)
	{
	    act("You hold $p in the air, a myriad of colours surround you.", ch, wand, NULL, TO_CHAR);
	    act("$n holds $p in the air, and there is a sudden flash of light.", ch, wand, NULL, TO_ROOM);
	    spell_cure_poison(skill_lookup("cure poison"), 20, ch, ch, TARGET_CHAR);
	    spell_cure_disease(skill_lookup("cure disease"), 20, ch, ch, TARGET_CHAR);
	    spell_heal(skill_lookup("heal"), 20, ch, ch, TARGET_CHAR);
	    spell_remove_curse(skill_lookup("remove curse"), 20, ch, ch, TARGET_CHAR);
	    if (dice(1, 6) > 1) extract_obj(wand);
	        else wand->value[0]++;
	    return;
	}
	else if (wand->value[0] == FEATHER_GOLDEN)
	{
	    ROOM_INDEX_DATA *room;
	    room = get_rand_room(ch);
	    act("$n holds $p in the air, and vanishes in a cloud of purple smoke.", ch, wand, NULL, TO_ROOM);
	    act("You hold $p in the air, and a cloud of purple smoke surrounds you.", ch, wand, NULL, TO_CHAR);
	    send_to_char("When the smoke clears you are somewhere different...\n\r", ch);
	    char_from_room(ch);
	    char_to_room(ch, room, 0);
	    act("$n appears in a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	    if (dice(1, 6) > 1) extract_obj(wand);
	        else wand->value[0]++;
	    return;
	}
	else if (wand->value[0] == FEATHER_PLATINUM)
	{
	    act("$n waves $p around.", ch, wand, NULL, TO_ROOM);
	    act("$n starts to grow wings, and transforms into a hawk.", ch, NULL, NULL,TO_ROOM);
	    send_to_char("Huge wings sprout from your back, and you shrink until you have become a hawk.\n\r", ch);
	    set_form(ch, FRM_HAWK);
	    REMOVE_BIT(ch->flag4, AFF_LANDED);
	    if (dice(1, 6) > 1) extract_obj(wand);
	        else wand->value[0] = FEATHER_COPPER;
	    return;
	}
	else
	{
	    send_to_char("The feather crumbles into dust.\n\r", ch);
	    act("$n waves $p about and it crumbles into dust.", ch, wand, NULL, TO_ROOM);
	    extract_obj(wand);
	    return;
	}
	return;
    }


    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	    act( "$n zaps you with $p.",ch, wand, victim, TO_VICT );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

	if (ch->level < wand->level 
	||  number_percent() >= 20 + get_skill(ch,gsn_wands)) 
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent();

    if (!IS_AWAKE(victim))
	percent -= 10;
    else if (!can_see(victim,ch))
	percent += 25;
    else 
	percent += 50;

    if ( ((ch->level + 7 < victim->level || ch->level -7 > victim->level) 
    && !IS_NPC(victim) && !IS_NPC(ch) )
    || ( !IS_NPC(ch) && percent > get_skill(ch,gsn_steal) + 10)
    || ( !IS_NPC(ch) && !is_clan(ch)) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops.\n\r", ch );
	affect_strip(ch,gsn_sneak);
	REMOVE_BIT(ch->affected_by,AFF_SNEAK);

	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
	case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
	}
	if (!IS_AWAKE(victim))
	    do_wake(victim,"");
	if (IS_AWAKE(victim))
	    do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
		check_improve(ch,gsn_steal,FALSE,2);
		set_fighting(victim, ch);
	    }
	    else
	    {
		sprintf(buf,"$N tried to steal from %s.",victim->name);
		wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
	    }
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) 
    ||   !str_cmp( arg1, "silver"))
    {
	int gold, silver;

	gold = victim->gold * number_range(1, ch->level) / 60;
	silver = victim->silver * number_range(1,ch->level) / 60;
	if ( gold <= 0 && silver <= 0 )
	{
	    send_to_char( "You couldn't get any coins.\n\r", ch );
	    return;
	}

	ch->gold        += gold;
	ch->silver      += silver;
	victim->silver  -= silver;
	victim->gold    -= gold;
	if (silver <= 0)
	    sprintf( buf, "Bingo!  You got %d gold coins.\n\r", gold );
	else if (gold <= 0)
	    sprintf( buf, "Bingo!  You got %d silver coins.\n\r",silver);
	else
	    sprintf(buf, "Bingo!  You got %d silver and %d gold coins.\n\r",
		    silver,gold);

	send_to_char( buf, ch );
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    free_string(obj->last_owner);
    obj->last_owner = str_dup(GET_PROPER_NAME(victim));
    act("You pocket $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_steal,TRUE,2);
    send_to_char( "Got it!\n\r", ch );
    return;
}

void do_listhouse(CHAR_DATA *ch, char *argument)
{
    if (rb_mob == NULL || rb_mob->in_room == NULL
    	|| rb_mob->in_room != ch->in_room)
    {
        stc("Bill the Brickie doesn't seem to be here.\n\r", ch);
        return;
    }

    if (argument[0] == 0 || str_cmp(argument, "auto"))
        do_say(ch, "I'm interested in the current housing costs.");
    act("$N shows a piece of paper to $n.", ch, NULL, rb_mob, TO_NOTVICT);
    stc("Billie the Brickie shows you a piece of paper, on which is written:\n\r", ch);
    cent_to_char("Current Housing Prices", ch);
    cent_to_char("-----=====><=====-----", ch); stc("\n\r", ch);
    cent_to_char("A new house = 50,000 Gold and takes 5 days to build.", ch);
    cent_to_char("A new room  = 20,000 Gold and takes 2 days to build.", ch);
    cent_to_char("A Lockable front door    =    200 gold.", ch);
    cent_to_char("An unpickable front door =    500 gold.", ch);
    cent_to_char("A Wizlocked front door   = 10,000 gold.", ch);
    cent_to_char("Having a fountain fitted will cost 10,000 Gold.", ch);
    stc("\n\rTo order type '#3buy <item>#n' where <item> can be any one of the\n\r",ch);
    stc("following:  #7house#n, #7extension#n, #7door#n, #7wizlocked#n, #7fountain#n\n\r", ch);
    do_say(ch, "Thankyou.");
}

void do_buyhouse(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *room, *room2;
    int type = 0, dir = 0, dir2 = 0, dir3 = 0, count = 0;
    char arg[MAX_INPUT_LENGTH], cdir[2], cdir2[2], cdir3[2];

    if (IS_NPC(ch) || ch->pcdata == NULL)
	return;

    if (rb_mob == NULL || rb_mob->in_room == NULL
    	|| rb_mob->in_room != ch->in_room)
    {
        stc("Bill the Brickie doesn't seem to be here.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (arg[0] == 0)
    {
        do_say(ch, "What kinds of houses do you sell?");
        type = 0;
    }
    else if (!str_prefix(arg, "house"))
    {
        do_say(ch, "I'd like to buy a new house please.");
        type = 1;
    }
    else if (!str_prefix(arg, "extension"))
    {
        do_say(ch, "I'd like to buy an extension please.");
        type = 2;
    }
    else if (!str_prefix(arg, "fountain"))
    {
        do_say(ch, "I'd like to purchase a fountain please.");
        type = 3;
    }
    else if (!str_prefix(arg, "door"))
    {
        do_say(ch, "I'd like a new front door please.");
        type = 4;
    }
    else if (!str_prefix(arg, "wizlocked"))
    {
        do_say(ch, "I'd like to buy a wizlocked front door please.");
        type = 5;
    }
    else
    {
        do_say(ch, "I'm not really sure what I want.");
        type = 6;
    }

    if (rb_mob->in_room->vnum != 20001 || rb_timer != -1)
    {
        do_say(rb_mob, "I'm a bit busy at the mo.  It would be best if you saw me in my office.");
    	return;
    }


    if (type == 0 || type == 6)
    {
        do_listhouse(ch, "auto");
        return;
    }

    if (rb_buildfor != NULL)
    {
	do_say(rb_mob, "I'm sorry but I'm already building something for someone else.");
	return;
    }

    if (type == 1)
    {
	char route[200];
	if (ch->pcdata->home > 0)
	{
	    do_say(rb_mob, "But you already own a house.  You know the mayor limits you to only one house.");
	    return;
	}

	if (TOTAL_WORTH(ch) < 50000 * CNV_GOL)
	{
	    do_say(rb_mob, "I'm affraid you don't have enough money.\n\r");
	    return;
	}

	give_cash(ch, rb_mob, 50000 * CNV_GOL);
	do_say(rb_mob, "Thankyou.  Now I'll take you to your new home.");

	REMOVE_BIT(ch->act, PLR_NOFOLLOW);
	if (ch->master != NULL)
	    stop_follower( ch );

	add_follower(ch, rb_mob);

	// find site
	rb_mob->autoroute = TRUE;
	rb_mob->routepos = 0;
	sprintf(route, "ws");
	room = get_room_index( prime_realm, 20002 );
	dir = dice(1, 3);
	if (dice(1, 2) == 1)
	     dir2 = dir - 1; 
	else dir2 = dir + 1;
	if (dir2 < 0) dir2 += 4;
	if (dir2 > 3) dir2 -= 4;
	dir3 = dir + 2;
	if (dir3 < 0) dir3 += 4;
	if (dir3 > 3) dir3 -= 4;
	cdir[0] = 's';  cdir2[0] = 's';  cdir3[0] = 's';
	cdir[1] = 0; 	cdir2[1] = 0;    cdir3[1] = 0;
	if (dir == 1) cdir[0] = 'e';
	if (dir == 3) cdir[0] = 'w';
	if (dir2 == 0) cdir2[0] = 'n';
	if (dir2 == 1) cdir2[0] = 'e';
	if (dir2 == 3) cdir2[0] = 'w';
	if (dir3 == 0) cdir3[0] = 'n';
	if (dir3 == 1) cdir3[0] = 'e';
	if (dir3 == 3) cdir3[0] = 'w';
	crash("Housing / OLC - finding Room");
	for ( ; ; )
	{
	    if (room->exit[dir] == NULL)
	    {
	 	if (olc_create_room(room, dir) == NULL)
		{
		    stc("BIG bug.  Out of rooms in area.  Report to Mitzrah.\n\r", ch);
		    return;
		}
		WRITE_STR(room->exit[dir]->u1.to_room->name, "Main Street.");
		WRITE_STR(room->exit[dir]->u1.to_room->description, "You are standing in the middle of one of the main connecting\n\rstreets in the housing estate.\n\r"); }
	    room = room->exit[dir]->u1.to_room;
	    strcat(route, cdir);
	    room2 = room;
	    for ( count = 0; count <= 10; count++)
	    {
	        if (room2->exit[dir2] == NULL)
	        {
		    if (olc_create_room(room, dir2) == NULL) 
		    {
		    	stc("BIG bug.  Out of rooms in area.  Report to Mitzrah.\n\r", ch);
		    	return;
		    }		
		    WRITE_STR(room2->exit[dir2]->u1.to_room->name, "Side Street.");
		    WRITE_STR(room2->exit[dir2]->u1.to_room->description, "You are standing in the side streets, where houses are actually situated.\n\r");
	        }
		room2 = room2->exit[dir2]->u1.to_room;
		if (room2->exit[dir3] == NULL)
		{
		    // NEW HOME FOUND!
		    int i;
		    char buf[1000];
		    if (olc_create_room(room2, dir3) == NULL) 
		    {
		    	stc("BIG bug.  Out of rooms in area.  Report to Mitzrah.\n\r", ch);
		    	return;
		    }		
		    sprintf(buf, "%s's home.", GET_PC_NAME(ch));
		    WRITE_STR(room2->exit[dir3]->u1.to_room->name, buf);
		    WRITE_STR(room2->exit[dir3]->u1.to_room->description, "You are standing in a new home, as it is being built.\n\r");
		    for (i = 0; i <= count; i++)
			strcat(route, cdir2);
		    strcat(route, cdir3);
		    strcat(route, "X");
		    WRITE_STR(rb_mob->route, route);
		    ch->pcdata->home = room2->exit[dir3]->u1.to_room->vnum;
		    rb_timer = 50;
		    rb_room = room2->exit[dir3]->u1.to_room;
		    WRITE_STR(rb_desc, "A brand spanking new room.");
		    olc_save_zone(rb_room->area, ch);
		    return;
		}
	    }
	}
	stc("No room for your house was found.  Report to Mitzrah", ch);
	return;
    }

    stc("That option, while recognised, has yet to be coded\n\r", ch);

}


void do_buy( CHAR_DATA *ch, char *argument )
{
    if (ch->in_room != NULL && ch->in_room->vnum == 20001)
    {
	do_buyhouse(ch, argument);
	return;
    }

    stc("No one is about to code this one.\n\r", ch);
    return;
}



void do_list( CHAR_DATA *ch, char *argument )
{

    if (ch->in_room != NULL && ch->in_room->vnum == 20001)
    {
	do_listhouse(ch, argument);
	return;
    }
    stc("No one is about to code this one.\n\r", ch);
    return;
}



void do_sell( CHAR_DATA *ch, char *argument )
{
}



void do_value( CHAR_DATA *ch, char *argument )
{
}


void do_activate(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );

    if ( arg1[0]=='\0' )
    {
	send_to_char("What do you wish to activate?\n\r",ch);
	return;
    }

    if ( (obj = get_obj_wear(ch, arg1)) == NULL)
    {
	send_to_char("You are not using that item.\n\r",ch);
	return;
    }

    if ( obj->spec_proc == 0 )
    {
	send_to_char("You cannot activate that.\n\r",ch);
	return;
    }

    switch (obj->spec_proc)
    {
	case 1 : item_spec_1(ch, argument);
		 break;
	case 2 : item_spec_2(ch, argument);
		 break;
	case 100: item_spec_100(ch, argument);
	default: bug("Special Procedure: no match.", 0 );
		 break;
    }
}

void item_spec_1(CHAR_DATA *victim, char *argument)
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( victim->in_room == NULL  
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( victim->fighting != NULL ))
    {
	send_to_char( "You failed.\n\r", victim );
	return;
    }

    pRoomIndex = get_random_room(victim);

    send_to_char("You have been teleported!\n\r",victim);
  
    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex, 0 );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void item_spec_2(CHAR_DATA *victim, char *argument)
{
	send_to_char("The ebony mask stares at you, its eyes glow red...", victim);
	SET_BIT(victim->flag2, AFF_FEAR);
}

void item_spec_100(CHAR_DATA *victim, char *argument)
{
    CHAR_DATA *vch;
	send_to_char("The blade of your sword radiates with the intense blackness of the Overlord\n\rof the Abyss, and Lord of Darkness, Blorgy-poo.\n\r",victim);
	act("The blade of $n's sword radiates with the intense blackness of the Overlord\n\rof the Abyss, and Lord of Darkness, Whimpy.\n\rYou are struck down by the tyrannical power held withing the blade.",victim,NULL,NULL,TO_ROOM);
	victim->hit = victim->max_hit;
	victim->move = victim->max_move;
	for (vch = victim->in_room->people; vch != NULL; vch = vch->next)
	{
	    if (vch != victim)
		destroy_char(vch, vch, FALSE);
	}
}

void do_scribe(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );

    if ( arg1[0]=='\0' )
    {
	send_to_char("What do you wish to scribe from?\n\r",ch);
	return;
    }

    if ( (obj = get_obj_wear(ch, arg1)) == NULL)
    {
	send_to_char("You are not using that item.\n\r",ch);
	return;
    }

    if ( obj->item_type != ITEM_SPELL_BOOK )
    {
	rand_to_char(ch,
	"That item doesn't contain enough magical energy to unzip your flies!\n\r",
	"I've seen more magic in horse shit!\n\r",
	"There's more magical potence in my Grandmothers underwear than in that.\n\r",
	"Thats about as magical as you are in bed.\n\r",
	"Theres more magical excellence in my little toe than in that.\n\r");
	return;
    }
}

