#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "magic.h"
#include "recycle.h"

/* command procedures needed */
DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_get_obj_world );

/*
 * Local functions.
 */
void    say_spell       args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void    wear_obj        args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool show ) );
void    exec_fear       args( ( CHAR_DATA *ch ) );
ROOM_INDEX_DATA * find_location args( ( CHAR_DATA *ch, char *arg ) );

void spell_instill_life( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *mob;

    if (obj == NULL)
    {
	stc("Animate which object?\n\r", ch);
	return;
    }

    mob = animate_object( obj );

    if (mob == NULL)
    {
	stc("The spell fails and is wasted.\n\r", ch);
	return;
    }

    act("You place your hands on $N.", ch, NULL, mob, TO_CHAR);
    act("$n places his hands on $N.", ch, NULL, mob, TO_ROOM);
    act("$n shudders with life.", mob, NULL, NULL, TO_ROOM);
}

void spell_haven( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    if (ch->diamonds < 1)
    {
	stc("You do not have the diamond required for this spell.\n\r", ch);
	stc("The spell energy is wasted.\n\r", ch);
	return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
	stc("You cannot set a recall within a safe room.\n\r", ch);
	return;
    }

    ch->recall_room = ch->in_room->vnum;
    ch->diamonds -= 1;
    act("$n places a diamond on the floor.", ch, NULL, NULL, TO_ROOM);
    act("The diamond flares and vanishes, consumed by the spell.", ch, NULL, NULL, TO_ROOM);
    stc("You draw upon the diamond to cast the spell.\n\r", ch);
    stc("The diamond vanishes.  You may now recall to this room.\n\r", ch);
}

void spell_nystul( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if (IS_OBJ_STAT2(obj, ITEM_NYSTUL))
    {
	act("$p is already enchanted with Nystul's magic aura.", ch, obj, NULL, TO_CHAR);
	return;
    }

    REMOVE_BIT(obj->extra_flags3, ITEM_HIDE_MAGIC);
    SET_BIT(obj->extra_flags2, ITEM_NYSTUL);
    act("$p glows with a false magical light.", ch, obj, NULL, TO_CHAR);
    act("$p, carried by $n glows with magical light.", ch, obj, NULL, TO_ROOM);
}

void spell_obscure_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if (IS_OBJ_STAT3(obj, ITEM_HIDE_MAGIC))
    {
	act("$p is already enchanted with Ged's magical obscurement.", ch, obj, NULL, TO_CHAR);
	return;
    }

    REMOVE_BIT(obj->extra_flags2, ITEM_NYSTUL);
    SET_BIT(obj->extra_flags3, ITEM_HIDE_MAGIC);
    act("Any magic in $p will now be obscured.", ch, obj, NULL, TO_CHAR);
}


void spell_vivify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (victim->hit == victim->max_hit)
    {
	stc("They are at full health already.\n\r", ch);
	return;
    }

    act("You vivify $N's body.", ch,NULL, victim, TO_CHAR);
    stc("You feel much better.\n\r", victim);
    stc("You feel much better.\n\r", ch);
    victim->hit += 100 + dice(1, 60);
    if (victim->hit > victim->max_hit) victim->hit = victim->max_hit;
    ch->hit += 100 + dice(1, 60);
    if (ch->hit > ch->max_hit) ch->hit = ch->max_hit;
}

void spell_wither( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int part = BODY_LEFT_ARM;

    if (victim->level == 120 || (victim->secondaryrace == 0 && !IS_NPC(victim))
	|| victim->water > ch->entropy
	|| saves_spell(ch->level, victim, DAM_OTHER) )
    {
	send_to_char("You feel a slight tingling in your arm.\n\r", victim);
	send_to_char("Nothing happens.\n\r", ch);
	return;
    }

    if (dice(1, 50) > 25)
	part = BODY_RIGHT_ARM;

    if (lose_body_part(victim, part))
    {
	sprintf(buf,"$N's arm withers and falls to the floor.");
	send_to_char("Your arm withers and drops to the floor.\n\r", victim);
	act(buf, ch, NULL, victim, TO_CHAR);
	set_fighting(ch, victim);
	return;
    }
    else
    {
	send_to_char("The spell failed.\n\r", ch);
	return;
    }
}

void spell_imbue( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_RESISTING(victim) && victim != ch)
    {
	stc("You can only do this on a willing subject.\n\r", ch);
	return;
    }

    if (victim->secondaryrace != 0)
    {
	stc("You cannot cast this spell on immortals.\n\r", ch);
	return;
    }

    taint_transform(victim, RACE_WARLOCK);
    stc("You suddenly feel knowledgable of magic.\n\r", victim);
    act("You imbue $N with spell ability.", ch, NULL, victim, TO_CHAR);
    act("$n mutters something to $N.", ch, NULL, victim, TO_NOTVICT);

    if (ch->power[DISC_WARL_LUCU] > 1)
	victim->power[DISC_WARL_LUCU] = ch->power[DISC_WARL_LUCU] / 2;
    if (ch->power[DISC_WARL_FARR] > 1)
	victim->power[DISC_WARL_FARR] = ch->power[DISC_WARL_FARR] / 2;
    if (ch->power[DISC_WARL_BREW] > 1)
	victim->power[DISC_WARL_BREW] = ch->power[DISC_WARL_BREW] / 2;
    if (ch->power[DISC_WARL_SCRI] > 1)
	victim->power[DISC_WARL_SCRI] = ch->power[DISC_WARL_SCRI] / 2;
    if (ch->power[DISC_WARL_ENCH] > 1)
	victim->power[DISC_WARL_SCRI] = ch->power[DISC_WARL_ENCH] / 2;
    if (ch->power[DISC_WARL_CONT] > 1)
	victim->power[DISC_WARL_CONT] = ch->power[DISC_WARL_CONT] / 2;
}

void spell_tree( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( ch->cur_form == FRM_TREE )
    {
	if (victim == ch)
	{
	    stc("But you are already in tree form.\n\r", ch);
	    return;
	}
	stc("They are already in the form of a tree.\n\r", ch);
	return;
    }

    if (IS_RESISTING(victim) && victim != ch)
    {
	stc("You can only do this on a willing subject.\n\r", ch);
	return;
    }

    SET_TIMER(victim, TIMER_TREE, dice(2, 4) );
    act("$n starts to grow and stiffen until $e is a tree.", victim, NULL, NULL, TO_ROOM);
    stc("You start to grow and stiffen and turn into a tree.\n\r", victim);
    set_form(victim, FRM_TREE);
}

void spell_growth( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_FLAG6(victim, AFF6_GROWTH) )
    {
	if (victim == ch)
	{
	    stc("You already have the growth spell on you!\n\r", ch);
	    return;
	}
	stc("They have already grown.\n\r", ch);
	return;
    }

    if (IS_FLAG6(victim, AFF6_SHRINK))
    {
	if (victim == ch)
  	{
	    stc("You grow back to your original size.\n\r", ch);
	    act("$n grows back up to $s original size.", ch, NULL, NULL, TO_ROOM);
	    affect_strip(ch, skill_lookup("shrink") );
            recalc_wear(victim);
	    return;
	}
	act("$n grows back to $s original size.", victim, NULL, NULL, TO_ROOM);
	stc("You grow back to normal.\n\r", victim);
	affect_strip(victim, skill_lookup("shrink"));
        recalc_wear(victim);
	return;
    }

    if (IS_RESISTING(victim) && victim != ch)
    {
	stc("You can only do this on a willing subject.\n\r", ch);
	return;
    }

    act("$n starts to grow.... to twice $s normal size.", victim, NULL, NULL, TO_ROOM);
    stc("You start to grow to twice your normal size.\n\r", victim);

    af.where     = TO_FLAG6;
    af.type      = sn;
    af.level     = ch->level;
    af.duration  = dice(3, 4);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF6_GROWTH;
    affect_to_char( victim, &af );
    recalc_wear(victim);
}

void spell_shrink( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_FLAG6(victim, AFF6_SHRINK) )
    {
	if (victim == ch)
	{
	    stc("You already have the shrink spell on you!\n\r", ch);
	    return;
	}
	stc("They have already shrunk.\n\r", ch);
	return;
    }

    if ( IS_FLAG6(victim, AFF6_GROWTH))
    {
	if (victim == ch)
  	{
	    stc("You shrink back down to your original size.\n\r", ch);
	    act("$n shrinks back down to $s original size.", ch, NULL, NULL, TO_ROOM);
	    affect_strip(ch, skill_lookup("growth") );
	    recalc_wear(victim);
	    return;
	}
	act("$n shrinks back to $s original size.", victim, NULL, NULL, TO_ROOM);
	stc("You shrink back to normal.\n\r", victim);
	affect_strip(victim, skill_lookup("growth") );
	recalc_wear(victim);
	return;
    }

    if (saves_spell(ch->level, victim, DAM_OTHER) && victim != ch)
    {
	stc("They are unaffected by the spell.\n\r", ch);
	return;
    }

    act("$n starts to shrink.... to half $s normal size.", victim, NULL, NULL, TO_ROOM);
    stc("You start to shrink to half your normal size.\n\r", victim);

    af.where     = TO_FLAG6;
    af.type      = sn;
    af.level     = ch->level;
    af.duration  = dice(3, 4);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF6_SHRINK;
    affect_to_char( victim, &af );
    recalc_wear(victim);
}

void spell_misfortune( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{ 
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SET(victim->flag5, AFF5_MISFORTUNE))
    {
	stc("Your victim is already unlucky.\n\r", ch);
	return;
    }
  
    if (victim->power[DISC_ELAD_MELO] >= 6)
    {
	stc("You cannot affect such a pure person.\n\r", ch);
  	stc("You defend against the evil magic.\n\r", victim);
	return;
    }

    af.where     = TO_FLAG5;
    af.type      = sn;
    af.level     = level;
    af.duration  = dice(1, 2);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF5_MISFORTUNE;
    affect_to_char( victim, &af );
}
void spell_time_sight( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int duration;

    if ( is_affected(victim, gsn_time_sight) )
    {
	if (victim == ch)
	{
	    stc("You already have that spell.\n\r", ch);
	    return;
	}
	stc("They already have that spell on them.\n\r", ch);
	return;
    }

    duration = dice(2, 4);

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = APPLY_AC;
    af.modifier  = -60;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = APPLY_HITROLL;
    af.modifier  = ch->air * 20;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = duration;
    af.location  = APPLY_DAMROLL;
    af.modifier  = ch->air * 20;
    af.bitvector = 0;
    affect_to_char( ch, &af );
 
    if (ch != victim)
   	stc("You bless them with time sight.\n\r", ch);

    stc("You gain acute visions of the near future.\n\r", victim);
}

void spell_ged_transform( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;

    af.where     = TO_FLAG5;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->earth + 3;
    af.location  = APPLY_AC;
    af.modifier  = -60;
    af.bitvector = AFF5_GED_TRANSFORM;
    affect_to_char( ch, &af );
    ch->rage = ch->earth * 15;
    SET_BIT(ch->flag5, AFF5_BEAST);
    stc("You unleash the beast within.\n\r", ch);
    act("$n starts growling and snarling angrily.", ch, NULL, NULL, TO_ROOM);
}


void spell_golum( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( ch->cur_form == FRM_GOLEM )
    {
	if (victim == ch)
	{
	    stc("But you are already in golem form.\n\r", ch);
	    return;
	}
	stc("They are already in the form of a golem.\n\r", ch);
	return;
    }

    if (IS_SET(victim->flag4, AFF_GEN_RESIST) && victim != ch)
    {
	stc("You can only do this on a willing subject.\n\r", ch);
	return;
    }

    SET_TIMER(victim, TIMER_GOLEM, dice(2, 4) );
    act("$n's skin starts to harden until $e is solid rock.", victim, NULL, NULL, TO_ROOM);
    stc("Your skin starts to harden until you turn into solid rock.\n\r", victim);
    set_form(victim, FRM_GOLEM);
}


void spell_jump( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_flying(victim) )
    {
        if (victim == ch)
          send_to_char("You are already airborne.\n\r",ch);
        else
          act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "You jump up into the air.\n\r", victim );
    act( "$n jumps high into the air.", victim, NULL, NULL, TO_ROOM );
    victim->position = POS_STANDING;

    REMOVE_BIT(victim->flag4, AFF_LANDED);
/*    if (can_go_up(victim)) victim->altitude += 1; */

    return;
}

void spell_air_barrier( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    AFFECT_DATA af;

    if ( IS_FLAG5(ch, AFF5_AIR_BARRIER) )
    {
        stc("You are already surrounded by an air barrier.",ch);
        return;
    }
    af.where     = TO_FLAG5;
    af.type      = sn;
    af.level     = level;
    af.duration  = dice(2, 3) - 1;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF5_AIR_BARRIER;
    affect_to_char( ch, &af );
    send_to_char( "A roaring barrier of air appears in front of you.\n\r", ch);
    act( "A roaring barrier of air appears in front of $n.", ch, NULL, NULL, TO_ROOM );

    return;
}

void spell_retribution( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_FLAG4(victim, AFF_RETRIBUTION) )
    {
        if (victim == ch)
          send_to_char("You already have this spell on you.\n\r",ch);
        else
          act("$N already has that spell.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_FLAG4;
    af.type      = sn;
    af.level     = level;
    af.duration  = 10+ch->fire;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_RETRIBUTION;
    affect_to_char( victim, &af );
    send_to_char( "A white aura forms around you.\n\r", victim );
    act( "A white aura forms around $n.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_REGENERATION) )
    {        
        send_to_char ("They already have that spell.\n\r", ch);
        return;
    }

    act( "$n lays his hands on $N.", ch, NULL, victim, TO_NOTVICT );

    af.where     = TO_NEWFLAGS;
    af.type      = sn;
    af.level     = level;
    af.duration  = ch->earth*2;
    if (ch->earth > 8)
        af.duration *= 2;       
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_REGENERATION;
    affect_to_char( victim, &af );
    send_to_char( "You body begins to heal at a remarkable rate.\n\r", victim );
    return;
}

void spell_accuracy( int sn, int level, CHAR_DATA *ch, void *vo, int target) 
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;


    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You already have this spell.\n\r",ch);
        else
          act("$N already has this spell.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (ch->air * 4);
    af.location  = APPLY_HITROLL;
    af.modifier  = ch->air + 10;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if (!IS_NPC(victim))
    {
        af.location  = APPLY_DAMROLL;
        af.modifier  = 0 - 15 + ch->air;
        af.bitvector = 0;
        affect_to_char( victim, &af );
    }
    send_to_char( "You now have an improved awareness of combat.\n\r", victim );
    if ( ch != victim )
        act("You grant $N accuracy in battle.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_pestilence( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_FLAG3(victim, AFF_PESTILENCE) )
    {
        send_to_char("They are already wracked with pestilence.\n\r", ch);
        return;
    }

    if (saves_spell(level,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
        if (ch == victim)
          send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
        else
          act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_FLAG3;
    af.type      = sn;
    af.level     = level * 3/5;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 0 - (ch->entropy / 2) + 3; 
    af.bitvector = AFF_PESTILENCE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as you are wracked with pestilence.\n\r",victim);
    act("$n screams in agony as $e is wracked with pestilence.",
        victim,NULL,NULL,TO_ROOM);
}

void spell_power_word_stun( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_FLAG3( victim, AFF_STUN ))
    {
        if (victim == ch)
          send_to_char("You are already stunned.\n\r",ch);
        else
          act("$N is already stunned.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (victim->level > ch->level)
    {
        send_to_char("They are too powerful to affect.\n\r", ch);
        send_to_char("You feel slightly dizzy for a moment.\n\r", victim);
        return;
    }

    af.where     = TO_FLAG3;
    af.type      = sn;
    af.level     = ch->fire * 10;
    af.duration  = 1;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector  = AFF_STUN;
    affect_to_char( victim, &af );
    send_to_char( "You are stunned by a resonating word of power.\n\r", victim );
    act("$n falls back, reeling and stunned.",victim,NULL,NULL,TO_ROOM);
    act("$N falls back, reeling and stunned.",victim,NULL,ch,TO_CHAR);
    return;
}

void spell_wall_of_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int *door = (int *) vo;
    char buf[MAX_INPUT_LENGTH];

    if (*door == DIR_UP || *door == DIR_DOWN)
    {
	send_to_char("You may only cast a wall of fire north, west, south or east.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("You cannot cast this spell in the air.\n\r", ch);
	return;
    }

    send_to_char("A huge wall of fire erupts from the ground.\n\r", ch);
    sprintf(buf, "A huge wall of fire erupts to the %s", dir_name[*door]);
    make_wall(ch->in_room, *door, EX_FIRE_WALL);
    act(buf, ch, NULL, NULL, TO_ROOM);
}

void spell_waterfall( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int *door = (int *) vo;
    char buf[MAX_INPUT_LENGTH];

    if (*door == DIR_UP || *door == DIR_DOWN)
    {
	send_to_char("You may only cast a waterfall north, west, south or east.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("You cannot cast this spell in the air.\n\r", ch);
	return;
    }

    send_to_char("A waterfall flows upwards from the ground.\n\r", ch);
    sprintf(buf, "A waterfall erupts from the ground to the %s", dir_name[*door]);
    make_wall(ch->in_room, *door, EX_WATER_WALL);
    act(buf, ch, NULL, NULL, TO_ROOM);
}

void spell_desecrate_exit( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int *door = (int *) vo;
    char buf[MAX_INPUT_LENGTH];

    send_to_char("You leave the mark of Baal.\n\r", ch);
    sprintf(buf, "$n points to the %s.", dir_name[*door]);
    REMOVE_BIT(ch->in_room->exit[*door]->exit_info, EX_WARDING);
    act(buf, ch, NULL, NULL, TO_ROOM);
}

void spell_stinking_cloud( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    ADD_RTIMER(ch->in_room, RTIMER_STINKING_CLOUD, dice(1, 4) + 1);
    room_message(ch->in_room, "Stinking green vapours start to fill the room.");
}

void spell_ghost_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    ADD_RTIMER(ch->in_room, RTIMER_GHOST_LIGHT, dice(1, 4) + 1);
    act("$n raises $s hands into the air.", ch, NULL, NULL, TO_ROOM);
    act("You raise your hands into the air.", ch, NULL, NULL, TO_CHAR);
    room_message(ch->in_room,"There is a blinding flash of light.");
    room_message(ch->in_room,"The air is suddenly filled with vapourous white howling spirits.");
}

void spell_consecrate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    ADD_RTIMER(ch->in_room, RTIMER_HIDE_ROOM, dice(1, 4) + 1);
    room_message(ch->in_room, "A shroud of night passes over the room.");
}

void spell_life_vortex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    room_message(ch->in_room, "A slight breeze in the room forms into a living vortex of energy.");
    if (!RTIME_UP(ch->in_room, RTIMER_LIFE_VORTEX) )
	room_message(ch->in_room, "The vortexes join as one.");
    ADD_RTIMER(ch->in_room, RTIMER_LIFE_VORTEX, dice(1, 4) + 1);
}

void spell_death_vortex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    room_message(ch->in_room, "A slight breeze in the room forms into a living vortex of energy.");

    if (!RTIME_UP(ch->in_room, RTIMER_LIFE_VORTEX) )
    {
	SET_TIMER(ch->in_room, RTIMER_LIFE_VORTEX, 0);
	SET_TIMER(ch->in_room, RTIMER_DEATH_VORTEX, 0);
	room_message(ch->in_room, "The vortex collapses upon itself.");
	return;
    }

    if (!RTIME_UP(ch->in_room, RTIMER_DEATH_VORTEX) )
	room_message(ch->in_room, "The vortexes join as one.");
    ADD_RTIMER(ch->in_room, RTIMER_DEATH_VORTEX, dice(1, 4) + 1);
}

void spell_wall_of_iron( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int *door = (int *) vo;
    char buf[MAX_INPUT_LENGTH];

    if (*door == DIR_UP)
    {
	send_to_char("You may only cast a wall of iron north, west, south, east or down.\n\r", ch);
	return;
    }

    if (ch->in_room->sector_type == SECT_AIR)
    {
	send_to_char("You cannot cast this spell in the air.\n\r", ch);
	return;
    }

    send_to_char("A huge wall of iron grows from the ground.\n\r", ch);
    sprintf(buf, "A huge wall of iron grows to the %s", dir_name[*door]);
    make_wall(ch->in_room, *door, EX_IRON_WALL);
    act(buf, ch, NULL, NULL, TO_ROOM);
}

void spell_illumination( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj;

    obj = create_object( get_obj_index( 86 + dice(1,3) ) , 0);

    obj_to_room(obj, ch->in_room, ch->altitude);
    SET_BIT(obj->extra_flags2, ITEM_FLYING);
    SET_BIT(ch->in_room->room_flags, ROOM_CONT_LIGHT);

    obj->timer = ch->fire + 5;

    act("$n twiddles $s thumbs and a ball of glowing light illuminates the room.", ch, NULL, NULL, TO_ROOM);    
    send_to_char("A huge ball of glowing light appears in the room.\n\r",ch);
}
