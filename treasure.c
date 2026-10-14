/*****************************************************************************
*									     *
*   AUTOMATIC TREASURE GENERATOR!!!!!  By          	                     *
*	Adam POWELL!!!!!!!!!!!!!!!!!!!!		  			     *
*									     *
*****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hell.h"
#include "recycle.h"

void improve_hitroll	args( (OBJ_DATA *obj, int amount) );
void improve_damroll	args( (OBJ_DATA *obj, int amount) );
void improve_ac		args( (OBJ_DATA *obj, int amount) );
void obj_improve_stat   args( (OBJ_DATA *obj, int stat, int amount) );

/* Moria eat your heart out... this is the biggest and best automatic treasure
generator in the world.  And its only compatible with HELL(TM) code */

/* Ok here are the random namebits, keep adding as many as you can!
    because these are random, the names dont signify anything, but with
    the weapons, 'of venom', or 'of the sun' will have a specific effect! */

#define MAX_RANDOM_FOOD 	11

struct random_food_type
{
    char * name;
    int	   item_type;
    int	   weight;
    int    v0;
    char * v1;
    char * v2;
    char * v3;
};

//     name     weight in 10th pounds   v0number  v1name  v2name  v3spellname
const struct random_food_type random_food_table [MAX_RANDOM_FOOD] =
{
    { "sausage", ITEM_FOOD, 3, 3, "three", NULL, NULL },
    { "melon", ITEM_FOOD, 30, 20, "twenty", NULL, NULL },
    { "loaf of bread", ITEM_FOOD, 0, 0, 0, 0, 0 },
    { "slice of pizza", ITEM_FOOD, 0, 0, 0, 0, 0 },
    { "beer", ITEM_DRINK_CON, 0, 0, 0, 0, 0 },
    { "", 0, 0, 0, 0, 0 },
    { "", 0, 0, 0, 0, 0 },
    { "", 0, 0, 0, 0, 0 },
    { "", 0, 0, 0, 0, 0 },
    { "", 0, 0, 0, 0, 0 },
    { NULL, 0, 0, 0, 0, 0 }
};

const char * potion_type[] =
{
    "glowing", "milky", "red", "green", "blue", "yellow", "pink",
    "white", "black", "dark blue", "cyan", "magenta", "dark green", 
    "metallic green", "silvery", "dark red", "violet", "indigo", "cloudy",
    "clear", "bubbling", "warm", "speckled", "rainbow", "brown",
    "metallic red", "metallic blue", "light red", "fizzy", "murky", ""
};

const char * staff_type[] =
{
    "wormwood", "mahogany", "elder", "yew", "sapling", "oak", "elm",
    "birch", "larch", "rainbow", "blackened", "silvery", "charcoal", 
    "fungus", ""
};

/* Ok it will be.... birch staff of fireballs (for instance) */

const char * staff_effect [] =
{
    "fireballs",  /* fireball */    
    "lightning",  /* lightning bolt */    
    "curing",  /* curing */    
    "healing",  /* spring of life */    
    "seeing",  /* cure blindness */    
    "blessing",  /* bless */    
    "lava storm",  /* lava burst and magma burst */    
    "indecision",  /* heal and harm */    
    "fire storm",  /* fireball and lava burst */    
    "fog",  /* faerie fog */    
    "grounding",  /* grounding */    
    "windtomb",  /* windtomb  */    
    "eyesight",  /* accuracy */    
    "anger",  /* cry of anger */    
    "blindness",  /* blindness */    
    "stones",  /* ballistic attack */    
    "storms",  /* call lightning */    
    "dispelling",  /* cancellation */    
    "soothing",  /* calm */    
    "insects",  /* cockroach */    
    "friends",  /* charm person */    
    "feasts",  /* heroes feast */    
    "lunch",  /* create food and create water */    
    "soothing",  /* remove fear and cure disease */    
    "cursing",  /* curse and demonfire */    
    "vision",  /* detect evil, good and magic */    
    "banishing",  /* dispel evil */    
    "draning",  /* energy drain */    
    "rapid fire",  /* fireball fireball and fireball */    
    "shocking",  /* ligbolt, ligbolt, ligbolt */    
    "escape",  /* invisiblity armor fly */    
    "madness",  /* frenzy */    
    "battle",  /* giant strenght and haste */    
    "famine",  /* hunger and thirst */    
    "humour",  /* hideous laughter */    
    "invulnerability",  /* sanct, armor, stonegrip */    
    "fire plague",  /* sumon flame x3 */    
    "withering",  /* wither */    
    ""
};

const char * scroll_title[] =
{
    "XIXAXA XOXAXA XUXAXA", "VENZAR BORGAVE", "ELAM EBOW", "FOOBIE BLETCH"
    "PRIRTUSIENE", ""
};

/* Quite improbable to get a colour, but what the hell */
const char * ring_colour[] =
{
    "frosted", "blue", "white", "black", "red", "orange", "yellow",
    "hot", "cold", "shiny", "blackened", "glowing", "green", "expensive",
    "cheap", "speckled", "vibrating", "glittering", "metallic", "dull",
    "pink", "icy", ""
};

/* Creates a blob that can be turned into something else */
OBJ_DATA * create_blob()
{
    OBJ_DATA *obj;
    obj = create_object( get_obj_index(98), 0);
    return obj;
}

void set_armor(OBJ_DATA *obj, int armour, bool magic)
{
    if (obj->item_type != ITEM_ARMOR) return;

    obj->value[0] = dice(1, armour) + armour / 2;
    obj->value[1] = dice(1, armour) + armour / 2;
    obj->value[2] = dice(1, armour) + armour / 2;
    if (magic) obj->value[3] = dice(1, armour) + armour / 2;
}

/* Only pass the APPLY_STR etc... to this one.. NOT STAT_STR */
void obj_improve_stat(OBJ_DATA *obj, int stat, int amount)
{
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR) return;

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = obj->level;
    af.duration     = -1;
    af.location     = stat;
    af.modifier	    = amount;
    af.bitvector    = 0;
    affect_to_obj(obj,&af);
}

void improve_ac(OBJ_DATA *obj, int amount)
{
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR) return;

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = obj->level;
    af.duration     = -1;
    af.location     = APPLY_AC;
    af.modifier	    = amount;
    af.bitvector    = 0;
    affect_to_obj(obj,&af);
}

void improve_hitroll(OBJ_DATA *obj, int amount)
{
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR) return;

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = obj->level;
    af.duration     = -1;
    af.location     = APPLY_HITROLL;
    af.modifier	    = amount;
    af.bitvector    = 0;
    affect_to_obj(obj,&af);
}

void improve_damroll(OBJ_DATA *obj, int amount)
{
    AFFECT_DATA af;

    if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR) return;

    af.where        = TO_OBJECT;
    af.type         = 0;
    af.level        = obj->level;
    af.duration     = -1;
    af.location     = APPLY_DAMROLL;
    af.modifier	    = amount;
    af.bitvector    = 0;
    affect_to_obj(obj,&af);
}

OBJ_DATA * create_random_food(int something)
{
    int food;

    food = dice(1, MAX_RANDOM_FOOD);

    // val 0 and 1 is the amount of food.  make both the same..  sod it    
    // if val 3 != 0 then its poisoned.  add more poisons !
    // value 2 is the mushroom effect
    // also, ITEM_PILL casts vals 1, 2 and 3 on the eater... ok?
    // if item_drink v0=capacity v1=curramount v2="liquid"

/*    yum = create_blob(); */ /* the blob has entered the building */

/*    yum->weight = random_food_table[food].weight;
    yum->item_type = random_food_table[food].item_type;
    yum->value[0] = random_food_table[food].v0;

    if (yum->item_type == ITEM_FOOD)
    {
    }
*/
    return NULL;
}

OBJ_DATA * create_random_bow(int something)
{
    OBJ_DATA *obj;
    int die;
    char before[200];
    char after[200];
    char totalname[200];

    obj = create_blob(); /* yay! the blob is here */
    before[0] = '\0';
    after[0] = '\0';

    obj->level = 1;
    obj->item_type = ITEM_BOW;
    obj->wear_flags = 0;
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    SET_BIT(obj->wear_flags, ITEM_WIELD);
    obj->condition = 100;
    obj->cost = 800;
    obj->weight = 35; /* all bows weigh nearly much */

    WRITE_STR(obj->material, "wood"); /* default is leather */

    /* All these can be 0 till I think of summing */
    obj->value[0] = 0;
    obj->value[1] = 0; /* Xtra Chance of hitting? 0 = normal, or up to 5! */
    obj->value[2] = 0; /* Spellbow?  If so set this value to the slot no. */
    obj->value[3] = 0; /* Strength value required to pull! */
    obj->value[4] = 0;

    die = dice(1, 50);	

    /* Firstly for the materials and similar shit */
    if (die <= 20)
    {
	WRITE_STR(obj->material, "wood");
	sprintf(before, "long");
	obj->size = SIZE_LARGE;
	obj->cost = 150 * CNV_GOL;
	obj->value[3] = 15;
    } 
    else if (die <= 40)
    {
	WRITE_STR(obj->material, "wood");
	sprintf(before, "short");
	obj->size = SIZE_MEDIUM;
	obj->cost = 90 * CNV_GOL;
	obj->value[3] = 13;
    } 
    else if (die <= 45)
    {
	WRITE_STR(obj->material, "wood");
	sprintf(before, "elegant ");
	obj->size = SIZE_LARGE;
	obj->cost = 190 * CNV_GOL;
	obj->value[1] = 3;
	obj->value[3] = 14;
    } 
    else if (die <= 46)
    {
	WRITE_STR(obj->material, "wood");
	sprintf(before, "tiny ");
	obj->size = SIZE_TINY;
	obj->cost = 190 * CNV_GOL;
	obj->value[3] = 6;
    } 
    else if (die <= 47)
    {
	WRITE_STR(obj->material, "mithril");
	sprintf(before, "mithril ");
	obj->size = SIZE_LARGE;
	obj->cost = 590 * CNV_GOL;
	obj->weight *= 2;
	obj->value[1] = 10;
	obj->value[3] = 16;
    } 
    else if (die <= 48)
    {
	WRITE_STR(obj->material, "adamanite");
	sprintf(before, "adamanite ");
	obj->size = SIZE_LARGE;
	obj->cost = 890 * CNV_GOL;
	obj->weight *= 2;
	obj->value[1] = 15;
	obj->value[3] = 18;
    } 
    else if (die <= 49)
    {
	WRITE_STR(obj->material, "ebony");
	sprintf(before, "ebony ");
	obj->size = SIZE_LARGE;
	obj->cost = 390 * CNV_GOL;
	obj->weight *= 4;
	obj->value[1] = -10;
	obj->value[3] = 18;
    } 
    else if (die <= 50)
    {
	WRITE_STR(obj->material, "gold");
	sprintf(before, "golden ");
	obj->size = SIZE_LARGE;
	obj->cost = 1390 * CNV_GOL;
	obj->weight /= 2;
	obj->value[1] = 15;
	obj->value[3] = 16;
    } 

    /* This can create the magical bows.   Yeah.
	value[1] = hit bonus   perm_spell!
	value[2] = spell       weight and cost
	value[3] = strength */
    if (dice(1, 20) == 1)
    {
	die = dice(1, 100);

	if (die <= 10)
	{
	    sprintf(after, "protection");
	    obj->perm_spell = ITEMA_SHIELD;	
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);
	    obj->cost *= 2;	    
	}
	else if (die <= 20)
	{
	    sprintf(after, "lightness");
	    obj->cost *= 2;	    
	    obj->weight /= 2;	    
	    obj->value[3] -= 4;
	}
	else if (die <= 30)
	{
	    sprintf(after, "healing");
	    obj->cost *= 3;	    
	    obj->value[3] -= 4;
	    obj->value[2] = skill_lookup("cure_light");
	    SET_BIT(obj->extra_flags, ITEM_GLOW);
	}
	else if (die <= 40)
	{
	    sprintf(after, "accuracy");
	    obj->cost *= 4;	    
	    obj->value[3] -= 2;
	    obj->perm_spell = ITEMA_ACCURACY;
	    obj->value[1] = 20;
	}
	else if (die <= 50)
	{
	    sprintf(after, "lightning");
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("lightning bolt");
	}
	else if (die <= 55)
	{
	    sprintf(after, "fireballs");
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);	    
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("fireball");
	}
	else if (die <= 60)
	{
	    sprintf(after, "ice strike");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("iceball");
	}
	else if (die <= 65)
	{
	    sprintf(after, "magic missiles");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("magic missile");
	}
	else if (die <= 70)
	{
	    sprintf(after, "extra healing");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("heal");
	}
	else if (die <= 75)
	{
	    sprintf(after, "shrouding");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("sanctuary");
	}
	else if (die <= 80)
	{
	    sprintf(after, "shrinking");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 4;	    
	    obj->value[2] = skill_lookup("shrink");
	}
	else if (die <= 85)
	{
	    sprintf(after, "growth");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 6;	    
	    obj->value[2] = skill_lookup("growth");
	}
	else if (die <= 90)
	{
	    sprintf(after, "plague");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 9;	    
	    obj->value[2] = skill_lookup("plague");
	}
	else if (die <= 95)
	{
	    sprintf(after, "the volcano");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    obj->cost *= 11;	    
	    obj->value[2] = skill_lookup("magma burst");
	}
	else if (die <= 100)
	{
	    sprintf(after, "goodness");
	    SET_BIT(obj->extra_flags, ITEM_MAGIC);	    
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	    obj->perm_spell = ITEMA_DETECT_EVIL;
	    obj->cost *= 12;	    
	    obj->value[2] = skill_lookup("dispel evil");
	}
    }

    /* Randomize the cost */
    obj->cost = (obj->cost / 2) + dice(1, obj->cost);

    /* Now string the names together... now we have all the 3 parts */
    if (after[0] == '\0')
    {
	sprintf(totalname, "a%s %sbow",
	    is_vowel(before[0]) ? "n" : "",
	    before);
	WRITE_STR(obj->short_descr, totalname);

	sprintf(totalname, "A%s %sbow lies here on the floor.",
	    is_vowel(before[0]) ? "n" : "",
	    before);
	WRITE_STR(obj->description, totalname);

	sprintf(totalname, "bow %sbow",
	    before);
	WRITE_STR(obj->name, totalname);
    }
    else
    {
	sprintf(totalname, "a%s %sbow of %s",
	    is_vowel(before[0]) ? "n" : "",
	    before, after);
	WRITE_STR(obj->short_descr, totalname);

	sprintf(totalname, "A%s %sbow lies here on the floor.",
	    is_vowel(before[0]) ? "n" : "",
	    before);
	WRITE_STR(obj->description, totalname);

	sprintf(totalname, "bow %sbow %s",
	    before, after);
	WRITE_STR(obj->name, totalname);
    }

    return obj;
}

OBJ_DATA * create_random_quiver(int something)
{
    OBJ_DATA *obj;
    char before[100];
    int die;
    char totalname[200];

    before[0] = '\0';

    obj = create_blob(); /* yay! the blob is here */

    obj->level = 1;
    obj->item_type = ITEM_QUIVER;
    obj->wear_flags = 0;
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    obj->condition = 100;
    obj->cost = 400;
    obj->weight = 15; /* all quivers weigh not much */

    WRITE_STR(obj->material, "leather"); /* default is leather */

    /* All these can be 0 till I think of summing */
    obj->value[0] = 0;
    obj->value[1] = 0;
    obj->value[2] = 0;
    obj->value[3] = 0;
    obj->value[4] = 0;

    die = dice(1, 20);	

    /* The first is my testy long bow, aha! */
    if (die <= 10)
    {
	WRITE_STR(obj->material, "leather");
	sprintf(before, "leather");
	obj->size = SIZE_LARGE;
	obj->cost = 150 * CNV_GOL;
	obj->value[3] = 15;
    } 
    else if (die <= 20)
    {
	WRITE_STR(obj->material, "iron");
	sprintf(before, "iron");
	obj->size = SIZE_MEDIUM;
	obj->cost = 90 * CNV_GOL;
	obj->value[3] = 13;
    } 

    /* Now string the names together... now we have all the 2 parts */

    sprintf(totalname, "a%s %s quiver",
        is_vowel(before[0]) ? "n" : "",
        before);
    WRITE_STR(obj->short_descr, totalname);

    sprintf(totalname, "A%s %s quiver lies here on the floor.",
        is_vowel(before[0]) ? "n" : "",
        before);
    WRITE_STR(obj->description, totalname);

    sprintf(totalname, "%s quiver",
        before);
    WRITE_STR(obj->name, totalname);

    return obj;
}

OBJ_DATA * create_random_arrow(int something)
{
    OBJ_DATA *obj;
    int die;
    char after[100];
    char before[100];
    char totalname[100];

    after[0] = '\0';
    before[0] = '\0';

    obj = create_blob(); /* yay! the blob is here */

    obj->item_type = ITEM_ARROW;
    obj->wear_flags = 0;
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    obj->condition = 100;
    obj->cost = 20;
    obj->weight = 4; /* all arrows weigh not much */

    WRITE_STR(obj->material, "wood"); /* default is wood */

    /* Now for level and armour value */
    obj->value[0] = 0;
    obj->value[1] = 1; /* Number of damage dice */
    obj->value[2] = 6; /* Type of damage dice */
    obj->value[3] = 0;
    obj->value[4] = 0; /* Funky effects !!!!
			   call check_funky_weapon with (ch, vict, arrow) */
    obj->value[5] = 0; /* Spell arrows!!!! number = slot of spell!
			   0 = no spell, -1 = slot 0 spell                */


    die = dice(1, 20);

    if (die <= 10)
    {
	WRITE_STR(obj->material, "wooden");
	sprintf(before, "wooden");
	obj->size = SIZE_MEDIUM;
	obj->cost = 10;
	obj->value[1] = 1;
	obj->value[2] = 6;
    }
    else if (die <= 20)
    {
	WRITE_STR(obj->material, "electricity");
	sprintf(before, "electric");
	obj->size = SIZE_MEDIUM;
	obj->cost = 100;
	obj->value[1] = 1;
	obj->value[2] = 6;
	SET_BIT(obj->value[4], WEAPON_SHOCKING);
    }

    /* Now string the names together... now we have all the 3 parts */
    if (after[0] == '\0')
    {
	sprintf(totalname, "a%s %s arrow",
	    is_vowel(before[0]) ? "n" : "",
	    before);
	WRITE_STR(obj->short_descr, totalname);

	sprintf(totalname, "A%s %s arrow lies here on the floor.",
	    is_vowel(before[0]) ? "n" : "",
	    before);
	WRITE_STR(obj->description, totalname);

	sprintf(totalname, "%s arrow",
	    before);
	WRITE_STR(obj->name, totalname);
    }
    else
    {
	sprintf(totalname, "a%s %s arrow of %s",
	    is_vowel(before[0]) ? "n" : "",
	    before, after);
	WRITE_STR(obj->short_descr, totalname);

	sprintf(totalname, "A%s %s arrow lies here on the floor.",
	    is_vowel(before[0]) ? "n" : "",
	    before);
	WRITE_STR(obj->description, totalname);

	sprintf(totalname, "%s arrow %s",
	    before, after);
	WRITE_STR(obj->name, totalname);
    }

    return obj;
}



OBJ_DATA * create_random_ring(int something)
{
    int die;
    OBJ_DATA *obj;
    char material[200];
    char totalname[200];

    /* Welcome to Janders ring emporium. 
	COMMON - chance 10
	UNCOMMON - chance 5
	RARE - chance 1 */

    material[0] = '\0';
    totalname[0] = '\0';

    obj = create_blob(); /* yay! the blob is here */

    obj->item_type = ITEM_ARMOR;
    obj->wear_flags = 0;
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    SET_BIT(obj->wear_flags, ITEM_WEAR_FINGER);
    obj->condition = 100;
    obj->weight = 2; /* all rings weigh not much */
    obj->value[5] = 0; /* Rubbing value */
    obj->value[6] = 0; /* Rubbing charges */

    /* Now for level and armour value */
    obj->value[0] = 0;
    obj->value[1] = 0;
    obj->value[2] = 0;
    obj->value[3] = 0;
    obj->value[4] = 0;

    /* Lets roll that dice and see the outcome....
	First the common non magical crap ones */

    /* Big replacement here - value 5 for rings is the RUBBING value
	eg the spell that will be cast when the ring is rubbed
	value[6] is the charges left in the ring */

    die = dice(1, 380);
  
    if (die <= 10)
    {
	sprintf(material, "wooden");	
	WRITE_STR(obj->material, "wood");
    }
    else if (die <= 20)
    {
	sprintf(material, "wedding");	
	WRITE_STR(obj->material, "gold");
	obj->cost = 400 + dice(3, 30);
    }
    else if (die <= 30)
    {
	sprintf(material, "amber");	
	WRITE_STR(obj->material, "amber");
	obj->value[5] = skill_lookup("fireball");
	obj->value[6] = 3;
	obj->cost = 50 * CNV_GOL;
    }
    else if (die <= 40)
    {
	sprintf(material, "emerald");	
	WRITE_STR(obj->material, "emerald");
	improve_hitroll(obj, dice(2, 2));
	obj->cost = 80 * CNV_GOL;
    }
    else if (die <= 50)
    {
	sprintf(material, "topaz");	
	WRITE_STR(obj->material, "topaz");
	obj->cost = 90 * CNV_GOL;
	improve_damroll(obj, dice(2, 2));
    }
    else if (die <= 60)
    {
	sprintf(material, "jewelled");	
	WRITE_STR(obj->material, "gold");
	obj->cost = 110 * CNV_GOL;
	improve_ac(obj, 0 - dice(1, 12) );
    }
    else if (die <= 70)
    {
	sprintf(material, "sapphire");	
	WRITE_STR(obj->material, "sapphire");
	obj->cost = 120 * CNV_GOL;
	obj->value[4] = 7 + dice(1, 3);
	improve_ac(obj, -1);
    }
    else if (die <= 80)
    {
	sprintf(material, "ebony");	
	WRITE_STR(obj->material, "ebony");
	obj->cost = 0;
	improve_ac(obj, dice(5, 5));
	SET_BIT(obj->extra_flags, ITEM_NODROP);
	SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
    }
    else if (die <= 90)
    {
	sprintf(material, "wire");	
	WRITE_STR(obj->material, "steel");
	obj->cost = 200 * CNV_GOL;
	SET_BIT(obj->extra_flags, ITEM_HUM);
	obj->value[6] = dice(3, 4);
	obj->value[5] = skill_lookup("heal");
    }
    else if (die <= 100)
    {
	sprintf(material, "tiger eye");	
	WRITE_STR(obj->material, "precious stones");
	obj->cost = 150 * CNV_GOL;
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	obj->value[6] = dice(3, 4);
	obj->value[5] = skill_lookup("cure blindness");
	improve_hitroll(obj, dice(2, 3) );
    }
    else if (die <= 110)
    {
	sprintf(material, "diamond");	
	WRITE_STR(obj->material, "diamond");
	SET_BIT(obj->extra_flags3, ITEM_STRUCTURE);
	obj->cost = 250 * CNV_GOL;
	obj->value[6] = dice(6, 4);
	obj->value[5] = skill_lookup("armor");
	improve_damroll(obj, dice(2, 3) );
    }
    else if (die <= 120)
    {
	sprintf(material, "blackened");	
	WRITE_STR(obj->material, "wood");
	obj->cost = 250 * CNV_GOL;
	obj->value[6] = dice(4, 4);
	obj->value[5] = skill_lookup("shield");
	improve_damroll(obj, dice(1, 3) );
    }
    else if (die <= 130)
    {
	sprintf(material, "frosty roseate");	
	WRITE_STR(obj->material, "glass");
	obj->cost = 200 * CNV_GOL;
	obj_improve_stat(obj, APPLY_STR, dice(1, 3) );
    }
    else if (die <= 140)
    {
	sprintf(material, "green glass");	
	WRITE_STR(obj->material, "glass");
	obj->cost = 200 * CNV_GOL;
	obj_improve_stat(obj, APPLY_INT, dice(1, 3) );
    }
    else if (die <= 150)
    {
	sprintf(material, "yellow glass");	
	WRITE_STR(obj->material, "glass");
	obj->cost = 200 * CNV_GOL;
	obj_improve_stat(obj, APPLY_WIS, dice(1, 3) );
    }
    else if (die <= 160)
    {
	sprintf(material, "rainbow");	
	WRITE_STR(obj->material, "glass");
	obj->cost = 200 * CNV_GOL;
	obj_improve_stat(obj, APPLY_CON, dice(1, 3) );
    }
    else if (die <= 170)
    {
	sprintf(material, "bat");	
	WRITE_STR(obj->material, "bats");
	obj->cost = 200 * CNV_GOL;
	obj_improve_stat(obj, APPLY_DEX, dice(1, 3) );
    }
    else if (die <= 180)
    {
	sprintf(material, "tooth");	
	WRITE_STR(obj->material, "teeth");
	obj->cost = 200 * CNV_GOL;
	obj_improve_stat(obj, APPLY_CHA, dice(1, 3) );
    }
    else if (die <= 190)
    {
	sprintf(material, "magnet");	
	WRITE_STR(obj->material, "iron");
	obj->cost = 290 * CNV_GOL;
	obj_improve_stat(obj, APPLY_CHA, dice(1, 5) );
	obj_improve_stat(obj, APPLY_HITROLL, 1);
    }
    else if (die <= 200)
    {
	sprintf(material, "iron");	
	WRITE_STR(obj->material, "iron");
	obj->cost = 290 * CNV_GOL;
	obj_improve_stat(obj, APPLY_DEX, dice(1, 5) );
	obj_improve_stat(obj, APPLY_DAMROLL, dice(1, 2) );
    }
    else if (die <= 210)
    {
	sprintf(material, "yellow and green");	
	WRITE_STR(obj->material, "steel");
	obj->cost = 280 * CNV_GOL;
	obj_improve_stat(obj, APPLY_CON, dice(1, 5) );
	obj_improve_stat(obj, APPLY_AC, 0 - dice(1, 4) );
    }
    else if (die <= 220)
    {
	sprintf(material, "gold ivy");	
	WRITE_STR(obj->material, "gold");
	obj->cost = 280 * CNV_GOL;
	obj_improve_stat(obj, APPLY_WIS, dice(1, 5) );
	obj_improve_stat(obj, APPLY_INT, dice(1, 4) );
    }
    else if (die <= 230)
    {
	sprintf(material, "straw");	
	WRITE_STR(obj->material, "straw");
	obj->condition = dice(2, 20);
	obj->perm_spell = ITEMA_BLESS;
	obj->cost = 190 * CNV_GOL;
	obj_improve_stat(obj, APPLY_INT, dice(1, 5) );
	SET_BIT(obj->extra_flags3, ITEM_STRUCTURE);
    }
    else if (die <= 240)
    {
	sprintf(material, "stone");	
	WRITE_STR(obj->material, "stone");
	obj->cost = 280 * CNV_GOL;
	obj_improve_stat(obj, APPLY_STR, dice(1, 5) );
	SET_BIT(obj->extra_flags, ITEM_GLOW);
    }
    else if (die <= 245)
    {
	sprintf(material, "varnished");	
	WRITE_STR(obj->material, "wood");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("lightning bolt");
	obj->value[6] = dice(3, 6);
    }
    else if (die <= 250)
    {
	sprintf(material, "ivy");	
	WRITE_STR(obj->material, "ivy");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("magma burst");
	obj->value[6] = dice(3, 6);
    }
    else if (die <= 255)
    {
	sprintf(material, "neon");	
	WRITE_STR(obj->material, "neon");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("illumination");
	obj->perm_spell = ITEMA_SHIELD;
	obj->value[6] = dice(3, 6);
    }
    else if (die <= 260)
    {
	sprintf(material, "windy");	
	WRITE_STR(obj->material, "wind");
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("windtomb");
	obj->value[6] = dice(1, 6);
    }
    else if (die <= 265)
    {
	sprintf(material, "evil");	
	WRITE_STR(obj->material, "evil");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("cause critical");
	obj->perm_spell = ITEMA_DETECT_GOOD;
	obj->value[6] = dice(3, 6);
    }
    else if (die <= 270)
    {
	sprintf(material, "green frog");	
	WRITE_STR(obj->material, "frogs");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("frog");
	obj->perm_spell = ITEMA_WATER_BREATHING;
	obj->value[6] = dice(1, 2);
    }
    else if (die <= 275)
    {
	sprintf(material, "silver");	
	WRITE_STR(obj->material, "silver");
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("charm person");
	obj->value[6] = dice(3, 6);
    }
    else if (die <= 280)
    {
	sprintf(material, "icy");	
	WRITE_STR(obj->material, "ice");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("chill touch");
	obj->value[6] = dice(5, 6);
    }
    else if (die <= 285)
    {
	sprintf(material, "colourful");	
	WRITE_STR(obj->material, "wood");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("colour spray");
	obj->value[6] = dice(3, 6);
    }
    else if (die <= 290)
    {
	sprintf(material, "bone");	
	WRITE_STR(obj->material, "bone");
	obj->cost = die * CNV_GOL;
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	obj->value[5] = skill_lookup("heroes feast");
	obj->value[6] = dice(1, 6);
    }
    else if (die <= 300)
    {
	sprintf(material, "silvery");	
	WRITE_STR(obj->material, "silver");
	obj->cost = die * CNV_GOL;
	SET_BIT(obj->extra_flags, ITEM_GLOW);
	obj->value[5] = skill_lookup("remove fear");
	obj->value[6] = dice(4, 6);
    }
    else if (die <= 310)
    {
	sprintf(material, "lead");	
	WRITE_STR(obj->material, "lead");
	obj->cost = die * CNV_GOL;
	obj->value[1] = 2;
	obj->value[2] = dice(1,3);
	obj->value[3] = 2;
	obj->value[0] = dice(1,2);
	obj->value[5] = skill_lookup("growth");
	obj->value[6] = dice(5,2);
	SET_BIT(obj->extra_flags3, ITEM_STRETCHY);
    }
    else if (die <= 320)
    {
	sprintf(material, "elastic");	
	WRITE_STR(obj->material, "elastic");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("shrink");
	obj->value[6] = dice(4,2);
	SET_BIT(obj->extra_flags3, ITEM_STRETCHY);
    }
    else if (die <= 330)
    {
	sprintf(material, "skull");	
	WRITE_STR(obj->material, "skull");
	obj->cost = die * CNV_GOL;
	obj->value[3] = dice(3, 4);
	obj->value[5] = skill_lookup("hunger");
	obj->value[6] = dice(3, 4);
    }
    else if (die <= 340)
    {
	sprintf(material, "petal");	
	WRITE_STR(obj->material, "petals");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("sanctuary");
	obj->value[6] = dice(3, 4);
	SET_BIT(obj->extra_flags3, ITEM_STRETCHY);
	obj->perm_spell = ITEMA_SANCTUARY;
    }
    else if (die <= 350)
    {
	sprintf(material, "floral");	
	WRITE_STR(obj->material, "petals");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("sleep");
	obj->value[6] = dice(3, 4);
	SET_BIT(obj->extra_flags3, ITEM_STRETCHY);
	obj->perm_spell = ITEMA_SANCTUARY;
    }
    else if (die <= 350)
    {
	sprintf(material, "lightning");	
	WRITE_STR(obj->material, "electricity");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("chain lightning");
	obj->value[6] = dice(1, 4);
	SET_BIT(obj->extra_flags3, ITEM_STRETCHY);
    }
    else if (die <= 360)
    {
	sprintf(material, "copper");	
	WRITE_STR(obj->material, "copper");
	obj->cost = die * CNV_GOL;
	obj->value[5] = skill_lookup("grounding");
	obj->value[6] = dice(5, 6);
    }
    else if (die <= 370)
    {
	sprintf(material, "platinum");	
	WRITE_STR(obj->material, "platinum");
	obj->cost = die * CNV_GOL * 2;
	set_armor(obj, 5, TRUE);
	obj->value[5] = skill_lookup("heal");
	obj->value[6] = dice(5, 6);
	obj->perm_spell = ITEMA_BLESS;
	SET_BIT(obj->extra_flags, ITEM_GLOW);
    }
    else if (die <= 380)
    {
	sprintf(material, "marble");	
	WRITE_STR(obj->material, "marble");
	obj->cost = die * CNV_GOL * 2;
	set_armor(obj, 7, TRUE);
	obj->value[5] = skill_lookup("slee");
	obj->value[6] = dice(2, 3);
	obj->perm_spell = ITEMA_BLESS;
	SET_BIT(obj->extra_flags, ITEM_HUM);
    }

    /* Add about 10 more spell rings, and then 20 V SPECIAL ones */


    /* This sets the name of the ring and returns the pointer */

    sprintf(totalname, "a%s %s ring",
        is_vowel(material[0]) ? "n" : "",
        material);
    WRITE_STR(obj->short_descr, totalname);

    sprintf(totalname, "A%s %s ring lies here on the floor.",
        is_vowel(material[0]) ? "n" : "",
        material);
    WRITE_STR(obj->description, totalname);

    sprintf(totalname, "%s ring",
        material);
    WRITE_STR(obj->name, totalname);

    return obj;
}

OBJ_DATA * create_random_weapon (int something)
{
    int die;
    OBJ_DATA *obj;
    char weaptype[60], prefix[60], suffix[60];
    char totalname[200];

    /* Now the BBIIGGG randomizing table.  I could use a struct, but I need
        the ability to add other things eg spec procs/descriptions, so I 
	wont */
  
    weaptype[0] = '\0';
    prefix[0] = '\0';
    suffix[0] = '\0';
    totalname[0] = '\0';

    obj = create_blob();

    obj->item_type = ITEM_WEAPON;
    obj->wear_flags = 0;
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    SET_BIT(obj->wear_flags, ITEM_WIELD);
    obj->condition = 100;

    obj->value[4] = 0; /* Just in case */

    /* This bit sets the kind of weapon */
    die = dice(1, 357); /* can be modified later */

    if (die <= 30)
    {
	sprintf(weaptype, "short sword");
	obj->value[0] = weapon_type("sword");
	obj->value[1] = 1;
	obj->value[2] = 6;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 20;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 60)
    {
	sprintf(weaptype, "sword");
	obj->value[0] = weapon_type("sword");
	obj->value[1] = 1;
	obj->value[2] = 8;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 40;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 90)
    {
	sprintf(weaptype, "axe");
	obj->value[0] = weapon_type("axe");
	obj->value[1] = 1;
	obj->value[2] = 8;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 30;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 120)
    {
	sprintf(weaptype, "whip");
	obj->value[0] = weapon_type("whip");
	obj->value[1] = 1;
	obj->value[2] = 3;
	obj->value[3] = attack_lookup("whip");
	obj->weight = 20;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 140)
    {
	sprintf(weaptype, "dagger");
	obj->value[0] = weapon_type("dagger");
	obj->value[1] = 1;
	obj->value[2] = 4;
	obj->value[3] = attack_lookup("stab");
	obj->weight = 10;
	obj->size = SIZE_SMALL;
    }
    else if (die <= 170)
    {
	sprintf(weaptype, "mace");
	obj->value[0] = weapon_type("mace");
	obj->value[1] = 2;
	obj->value[2] = 4;
	obj->value[3] = attack_lookup("pound");
	obj->weight = 55;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 200)
    {
	sprintf(weaptype, "flail");
	obj->value[0] = weapon_type("flail");
	obj->value[1] = 2;
	obj->value[2] = 3;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 60;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 230)
    {
	sprintf(weaptype, "staff");
	obj->value[0] = weapon_type("staff");
	obj->value[1] = 1;
	obj->value[2] = 6;
	obj->value[3] = attack_lookup("smash");
	obj->weight = 20;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 250)
    {
	sprintf(weaptype, "quarterstaff");
	obj->value[0] = weapon_type("staff");
	obj->value[1] = 1;
	obj->value[2] = 8;
	obj->value[3] = attack_lookup("smash");
	obj->weight = 60;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 270)
    {
	sprintf(weaptype, "longsword");
	obj->value[0] = weapon_type("sword");
	obj->value[1] = 1;
	obj->value[2] = 10;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 60;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 290)
    {
	sprintf(weaptype, "greatsword");
	obj->value[0] = weapon_type("sword");
	obj->value[1] = 2;
	obj->value[2] = 8;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 90;
	obj->size = SIZE_LARGE;
    }
    else if (die <= 310)
    {
	sprintf(weaptype, "spear");
	obj->value[0] = weapon_type("polearm");
	obj->value[1] = 1;
	obj->value[2] = 9;
	obj->value[3] = attack_lookup("stab");
	obj->weight = 40;
	obj->size = SIZE_LARGE;
    }
    else if (die <= 320)
    {
	sprintf(weaptype, "halberd");
	obj->value[0] = weapon_type("polearm");
	obj->value[1] = 1;
	obj->value[2] = 12;
	obj->value[3] = attack_lookup("stab");
	obj->weight = 90;
	obj->size = SIZE_LARGE;
    }
    else if (die <= 330)
    {
	sprintf(weaptype, "great axe");
	obj->value[0] = weapon_type("axe");
	obj->value[1] = 3;
	obj->value[2] = 4;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 150;
	obj->size = SIZE_LARGE;
    }
    else if (die <= 340)
    {
	sprintf(weaptype, "glaive");
	obj->value[0] = weapon_type("polearm");
	obj->value[1] = 2;
	obj->value[2] = 8;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 95;
	obj->size = SIZE_LARGE;
    }
    else if (die <= 350)
    {
	sprintf(weaptype, "hand axe");
	obj->value[0] = weapon_type("axe");
	obj->value[1] = 1;
	obj->value[2] = 6;
	obj->value[3] = attack_lookup("slash");
	obj->weight = 40;
	obj->size = SIZE_MEDIUM;
    }
    else if (die <= 355)
    {
	sprintf(weaptype, "maul");
	obj->value[0] = weapon_type("mace");
	obj->value[1] = 3;
	obj->value[2] = 8;
	obj->value[3] = attack_lookup("pound");
	obj->weight = 250;
	obj->size = SIZE_LARGE;
    }
    else if (die <= 357)
    {
	sprintf(weaptype, "great maul");
	obj->value[0] = weapon_type("mace");
	obj->value[1] = 3;
	obj->value[2] = 12;
	obj->value[3] = attack_lookup("pound");
	obj->weight = 450;
	obj->size = SIZE_HUGE;
    }


    /* Weapon materials
	1 to 500 is common
	500 to 1000 is uncommon 
	1000 to 1100 is RARE   - adjust the dice as fit */

    die = dice(1, 500);

    if (dice(1, 3) == 1) die = dice(1, 700);

    /* Wooden weapons - no extra bonuses */
    if (die <= 100)
    {
	WRITE_STR(obj->material, "wood");
	sprintf(prefix, "wooden");
  	obj->weight /= 2;
	obj->value[3] = attack_lookup("smash");
	obj->value[1] = UMAX(obj->value[1] - 1, 1);
	obj->value[2] = UMAX(obj->value[2] - 2, 1);
    }
    else if (die <= 150) /* Bone weapons are slightly better than wooden */
    {
	WRITE_STR(obj->material, "bone");
	obj->weight -= 5;
	sprintf(prefix, "bone");
	obj->value[1] = UMAX(obj->value[1] - 1, 1);
	obj->value[2] = UMAX(obj->value[2] - 1, 1);
    }
    else if (die <= 190)
    {
	WRITE_STR(obj->material, "ivory");
	sprintf(prefix, "ivory");
	obj->value[2] = UMAX(obj->value[2] - 1, 1);
    }
    else if (die <= 250)
    {
	WRITE_STR(obj->material, "stone");
	sprintf(prefix, "stone");
	obj->weight *= 3; /* Stone weapons are very heavy! */
	obj->value[2] += 2;
	obj->value[1] = UMAX(obj->value[1] - 1, 1);
    }
    else if (die <= 270)
    {
	WRITE_STR(obj->material, "stone");
	sprintf(prefix, "flint");
	obj->weight *= 2; /* Stone weapons are very heavy! */
	obj->value[1]++;
	obj->value[2] = UMAX(obj->value[2] - 1, 1);
    }
    else if (die <= 370)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "steel");
    }
    else if (die <= 420)
    {
	WRITE_STR(obj->material, "brass");
	sprintf(prefix, "brass");
	obj->weight -= 2;
    }
    else if (die <= 430)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "elven steel");
	obj->weight /= 2;
	obj->value[2]++;
	improve_hitroll(obj, 1);
    }
    else if (die <= 480)
    {
	WRITE_STR(obj->material, "ebony");
	sprintf(prefix, "ebony");
  	obj->weight *= 2;
  	obj->weight /= 3;
	obj->value[2]++;
	obj->value[3] = attack_lookup("smash");
	obj->value[1] = UMAX(obj->value[1] - 1, 1);
    }
    else if (die <= 520)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "tarnished steel");
	improve_damroll(obj, -1);
    }
    else if (die <= 540)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "rusty steel");
	improve_damroll(obj, -1);
    }
    else if (die <= 560)
    {
	WRITE_STR(obj->material, "copper");
	sprintf(prefix, "copper");
	improve_hitroll(obj, -3);
    }
    else if (die <= 570)
    {
	WRITE_STR(obj->material, "silver");
	sprintf(prefix, "silver");
	obj->value[2]++;
	SET_BIT(obj->value[4], WEAPON_SILVER); /* poor weres */
    }
    else if (die <= 580) /* This will fuck the cybermen! */
    {
	WRITE_STR(obj->material, "gold");
	sprintf(prefix, "gold");
	obj->value[2]++;
	obj->value[1]++;
	improve_damroll(obj, dice(1, 5) );
	improve_hitroll(obj, dice(1, 5) );
    }
    else if (die <= 590)
    {
	WRITE_STR(obj->material, "adamanite");
	sprintf(prefix, "adamanite");
	obj->value[2]++;
	obj->value[1]++;
	improve_damroll(obj, dice(2, 5) );
	improve_hitroll(obj, dice(2, 5) );
	improve_ac(obj, dice(3, 5) );
    }
    else if (die <= 600)
    {
	WRITE_STR(obj->material, "mithril");
	sprintf(prefix, "mithril");
	improve_damroll(obj, dice(2, 4) );
	improve_hitroll(obj, dice(2, 4) );
	improve_ac(obj, dice(3, 4) );
	obj_improve_stat(obj, APPLY_STR, 1);
	SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
    }
    else if (die <= 600)
    {
	WRITE_STR(obj->material, "lava");
	sprintf(prefix, "lava");
	improve_damroll(obj, dice(2, 3) );
	improve_hitroll(obj, dice(2, 3) );
	SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	obj_improve_stat(obj, APPLY_WIS, 1);
	SET_BIT(obj->extra_flags, ITEM_GLOW);
    }
    else if (die <= 610)
    {
	WRITE_STR(obj->material, "iron");
	sprintf(prefix, "cold iron");
	improve_damroll(obj, -2);
	improve_hitroll(obj, -2);
	SET_BIT(obj->extra_flags3, ITEM_IRON);
    }
    else if (die <= 620)
    {
	WRITE_STR(obj->material, "iron");
	sprintf(prefix, "wrought iron");
	improve_damroll(obj, -1);
	improve_hitroll(obj, -1);
	SET_BIT(obj->extra_flags3, ITEM_IRON);
    }
    else if (die <= 630)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "soldiers");
	improve_damroll(obj, dice(2, 5) );
	improve_hitroll(obj, dice(2, 5) );
	improve_ac(obj, 10 + dice(1, 10) );
	obj->value[1]++;
	obj->value[2]++;
    }
    else if (die <= 640)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "demonic steel");
	improve_damroll(obj, dice(2, 8) );
	improve_hitroll(obj, dice(2, 8) );
	improve_ac(obj, 20 + dice(1, 10) );
	obj->value[1]++;
	obj->value[2]++;
	SET_BIT(obj->extra_flags, ITEM_FLAMING);
	SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	SET_BIT(obj->extra_flags, ITEM_EVIL);
    }
    else if (die <= 650)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "flaming");
	improve_ac(obj, dice(1, 10) );
	obj->value[1]++;
	obj->value[2]++;
	SET_BIT(obj->extra_flags, ITEM_FLAMING);
	SET_BIT(obj->value[4], WEAPON_FLAMING);
    }
    else if (die <= 660)
    {
	WRITE_STR(obj->material, "stone");
	sprintf(prefix, "solid");
	improve_ac(obj, 0-dice(3, 5) );
	improve_damroll(obj, dice(2, 6) );
	SET_BIT(obj->extra_flags3, ITEM_STRUCTURE);
    }
    else if (die <= 670)
    {
	WRITE_STR(obj->material, "wood");
	sprintf(prefix, "damaged wooden");
	obj->value[2] = UMAX(obj->value[2] - 1, 1);
	obj->value[1] = UMAX(obj->value[1] - 1, 1);
	obj->weight /= 2;
	obj->condition = 30;
    }
    else if (die <= 680)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "damaged steel");
	obj->condition = 30;
    }
    else if (die <= 690)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "titans");
	obj->value[1] += 2;
	obj->value[2] += 2;
	obj->weight *= 3;
    }
    else if (die <= 700)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "shocking");
	obj->value[1] += 1;
	obj->value[2] += 1;
	SET_BIT(obj->value[4], WEAPON_SHOCKING);
    }
    else if (die <= 710)
    {
	WRITE_STR(obj->material, "steel");
	sprintf(prefix, "acidic");
	obj->value[1] += 1;
	obj->value[2] += 1;
	SET_BIT(obj->value[4], WEAPON_POISON);
    }
    else if (die <= 720)
    {
	sprintf(prefix, "hardwood");
	WRITE_STR(obj->material, "wood");
	obj->value[1] += 2;
	obj->value[2] += 1;
	SET_BIT(obj->extra_flags3, ITEM_STRUCTURE);
    }

    /* Doesnt do much, but why the hell not */
    obj->cost = die / 10;

    /* Now for the suffix.   Muhahahahahaha, these are VERY special items */
    if (dice(1, 25) == 1)
    {
   	/* Got to this point.... Ok.
	    COMMON = 1 - 400,
	    UNCOMMON = 400 - 470
	    RARE = 471 - 500 */

	die = dice(1, 500);

  	/* All these weapons are well hard and powerful */
	obj->value[1]++;
	obj->value[2]++;
	SET_BIT(obj->extra_flags3, ITEM_STRUCTURE);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);
	SET_BIT(obj->extra_flags, ITEM_NOLOCATE);
	obj->cost = 50000; /* minimum cost is 500 gold! */

	obj->cost = 50000 + dice(1,2000 * die / 3);

	/* Uncommon weapon bonuses */
	if (die >= 400)
	{
	    obj->value[1]++;
	    obj->value[2]++;
	}

	/* Rare weapon bonuses */
	if (die >= 470)
	{
	    obj->value[1]++;
	    obj->value[2]++;
	    obj->weight /= 2;
	}

	if (die <= 20)
 	{
	    sprintf(suffix, "defending");
	    improve_ac(obj, 0 - dice(5, 5) );
	}
	else if (die <= 40)
 	{
	    sprintf(suffix, "speed");
	    obj->value[6] = 5;
	}
	else if (die <= 60)
 	{
	    sprintf(suffix, "great reach");
	    obj->value[7] = 8;
	}
	else if (die <= 80)
 	{
	    sprintf(suffix, "accuracy");
	    improve_hitroll(obj, dice(4, 4));
	}
	else if (die <= 100)
 	{
	    sprintf(suffix, "extra damage");
	    improve_damroll(obj, dice(4, 4));
	}
	else if (die <= 120)
 	{
	    sprintf(suffix, "obscene damage");
	    improve_damroll(obj, dice(4, 8));
	}
	else if (die <= 140)
 	{
	    sprintf(suffix, "weightlessness");
	    obj->weight = 0;
	    SET_BIT(obj->extra_flags2, ITEM_FLYING);
	    SET_BIT(obj->extra_flags, ITEM_GLOW);
   	    obj->value[6] = 8;
	}
	else if (die <= 160)
 	{
	    sprintf(suffix, "life draining");
	    SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	    SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
	    SET_BIT(obj->value[4], WEAPON_VAMPIRIC);
	    improve_damroll(obj, 10);
	    improve_hitroll(obj, 10);
	}
	else if (die <= 180)
 	{
	    sprintf(suffix, "demon smiting");
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	    SET_BIT(obj->value[4], WEAPON_SMITE_DEMONS);
	    SET_BIT(obj->extra_flags, ITEM_HUM);
	    obj->value[1]++;
	    obj->value[2]++;
	}
	else if (die <= 200)
 	{
	    sprintf(suffix, "mage killing");
	    SET_BIT(obj->value[4], WEAPON_POISON);
	    SET_BIT(obj->value[4], WEAPON_SMITE_WARLOCKS);
	    SET_BIT(obj->extra_flags, ITEM_HUM);
	    obj->value[1]++;
	    obj->value[2]++;
	}
	else if (die <= 220)
 	{
	    sprintf(suffix, "dragon killing");
	    SET_BIT(obj->value[4], WEAPON_POISON);
	    SET_BIT(obj->value[4], WEAPON_SMITE_DRAGONS);
	    SET_BIT(obj->extra_flags, ITEM_HUM);
	    obj->value[1]++;
	    obj->value[2]++;
	}
	else if (die <= 240)
 	{
	    sprintf(suffix, "eladrin murdering");
	    SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	    SET_BIT(obj->value[4], WEAPON_POISON);
	    SET_BIT(obj->value[4], WEAPON_SMITE_ELADRIN);
	    SET_BIT(obj->extra_flags, ITEM_HUM);
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	    obj->value[1]++;
	    obj->value[2]++;
	}
	else if (die <= 260)
 	{
	    sprintf(suffix, "water breathing");
	    obj->perm_spell = ITEMA_WATER_BREATHING;
	}
	else if (die <= 280)
 	{
	    sprintf(suffix, "plaguing");
	    obj->perm_spell = ITEMA_PLAGUE;
	    obj->cost = 0;
	}
	else if (die <= 300)
 	{
	    sprintf(suffix, "uselessness");
	    obj->value[1] = 1;
	    obj->value[2] = 1;
	    obj->weight = 100;
	    obj->condition = dice(3, 20);
	    obj->cost = 0;
	    improve_damroll(obj, -50);
	    improve_hitroll(obj, -50);
	    SET_BIT(obj->extra_flags, ITEM_NODROP);
	    SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
	}
	else if (die <= 320)
 	{
	    sprintf(suffix, "sanctuary");
	    obj->perm_spell = ITEMA_SANCTUARY;
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	    improve_damroll(obj, -5);
	}
	else if (die <= 330)
 	{
	    sprintf(suffix, "stone skin");
	    obj->perm_spell = ITEMA_STONE_SKIN;
	    improve_hitroll(obj, 0-dice(1, 5));
	    improve_damroll(obj, -5);
	}
	else if (die <= 340)
 	{
	    sprintf(suffix, "evil sniffing");
	    obj->perm_spell = ITEMA_DETECT_EVIL;
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	}
	else if (die <= 350)
 	{
	    sprintf(suffix, "good sniffing");
	    obj->perm_spell = ITEMA_DETECT_GOOD;
	    SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	    obj_improve_stat(obj, APPLY_INT, 1);
	}
	else if (die <= 360)
 	{
	    sprintf(suffix, "warfare");
	    obj->perm_spell = ITEMA_ARMOR;
	    obj->value[1] += 2;
	    obj->value[2] += 2;
	    improve_hitroll(obj, 5+dice(1, 10));
	    improve_damroll(obj, 5+dice(1, 10));
	    improve_ac(obj, -10);
	    obj_improve_stat(obj, APPLY_STR, 3);
	    SET_BIT(obj->extra_flags, ITEM_GLOW);
	}
	else if (die <= 360)
 	{
	    sprintf(suffix, "stupidity");
	    SET_BIT(obj->extra_flags, ITEM_NODROP);
	    SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
	    obj_improve_stat(obj, APPLY_INT, 0 - dice(4, 4) );
	}
	else if (die <= 370)
 	{
	    sprintf(suffix, "leadership");
	    obj_improve_stat(obj, APPLY_CHA, 8 );
	    SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	    improve_ac(obj, -40);
	}
	else if (die <= 380)
 	{
	    sprintf(suffix, "fiery death");
	    SET_BIT(obj->value[4], WEAPON_FLAMING);
	    SET_BIT(obj->value[4], WEAPON_VAMPIRIC);
	    SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	    obj_improve_stat(obj, APPLY_STR, 6 );
	    SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	    improve_ac(obj, -20-dice(1, 10));
	}
	else if (die <= 390)
 	{
	    sprintf(suffix, "frost brand");
	    SET_BIT(obj->value[4], WEAPON_FROST);
	    SET_BIT(obj->value[4], WEAPON_VORPAL);
	    obj_improve_stat(obj, APPLY_CON, 3 );
	    obj_improve_stat(obj, APPLY_STR, 3 );
	    SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	    improve_hitroll(obj, dice(2, 8));
	    improve_damroll(obj, dice(2, 8));
	    improve_ac(obj, -20-dice(1, 10));
	}
	else if (die <= 400)
 	{
	    sprintf(suffix, "the moon");
	    SET_BIT(obj->value[4], WEAPON_SILVER);
	    SET_BIT(obj->value[4], WEAPON_VORPAL);
	    obj_improve_stat(obj, APPLY_CON, 4 );
	    obj_improve_stat(obj, APPLY_STR, 4 );
	    SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	    obj->value[1]++;
	    obj->value[2]++;
	    improve_hitroll(obj, dice(3, 8));
	    improve_damroll(obj, dice(3, 8));
	    improve_ac(obj, -20-dice(2, 20));
	}
	else if (die <= 405)
 	{
	    sprintf(suffix, "intelligence");
	    obj_improve_stat(obj, APPLY_INT, dice(1,8) );
	    SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	}
	else if (die <= 410)
 	{
	    sprintf(suffix, "wisdom");
	    obj_improve_stat(obj, APPLY_WIS, dice(1,8) );
	    SET_BIT(obj->extra_flags2, ITEM_LOYAL);
	}
	else if (die <= 415)
 	{
	    sprintf(suffix, "first attack");
	    obj_improve_stat(obj, APPLY_DEX, dice(1,8) );
	    improve_hitroll(obj, dice(3, 10) );
	    obj->perm_spell = ITEMA_HASTE;
	    obj->value[6] = 4;
	}
	else if (die <= 420)
 	{
	    sprintf(suffix, "far reaching");
	    obj_improve_stat(obj, APPLY_DEX, dice(1,6) );
	    improve_hitroll(obj, dice(3, 10) );
	    obj->perm_spell = ITEMA_SHIELD;
	    obj->value[7] = 10;
	}
	else if (die <= 425)
 	{
	    sprintf(suffix, "vanishing");
	    obj_improve_stat(obj, APPLY_DEX, 1 );
	    obj_improve_stat(obj, APPLY_INT, 1 );
	    improve_ac(obj, 0 - dice(4, 10) );
	    obj->perm_spell = ITEMA_INVISIBILITY;
	}
	else if (die <= 430)
 	{
	    sprintf(suffix, "shortness");
	    obj->value[7] = 2;
	    obj->cost = 0;
	}
	else if (die <= 435)
 	{
	    sprintf(suffix, "demonic vengance");
	    SET_BIT(obj->value[4], WEAPON_SMITE_ELADRIN);
	    SET_BIT(obj->value[4], WEAPON_FLAMING);
	    SET_BIT(obj->value[4], WEAPON_VAMPIRIC);
	    SET_BIT(obj->value[4], WEAPON_FROST);
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	    SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	    SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
	    obj->value[2] += 3;
	    obj->value[6] = 8;
	    obj->cost *= 2;
	}
	else if (die <= 440)
 	{
	    sprintf(suffix, "enlightenment");
	    obj_improve_stat(obj, APPLY_INT, dice(1, 6) );
	    obj_improve_stat(obj, APPLY_WIS, dice(1, 6) );
	    improve_hitroll(obj, dice(3, 10) );
	    obj->perm_spell = ITEMA_DETECT_MAGIC;
	}
	else if (die <= 450)
 	{
	    sprintf(suffix, "bless");
	    improve_hitroll(obj, dice(3, 10) );
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	    obj->perm_spell = ITEMA_BLESS;
	}
	else if (die <= 460)
 	{
	    sprintf(suffix, "strength");
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	    obj_improve_stat(obj, APPLY_STR, dice(2, 3));
	    improve_damroll(obj, dice(3, 5) );
	}
	else if (die <= 465)
 	{
	    sprintf(suffix, "the hurricane");
	    obj->perm_spell = ITEMA_WINDTOMB;
	}
	else if (die <= 470)
 	{
	    sprintf(suffix, "the forge");
	    obj->perm_spell = ITEMA_IRON_SKIN;
	    obj_improve_stat(obj, APPLY_HIT, dice(2, 100));
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	    SET_BIT(obj->value[4], WEAPON_FLAMING);
	}
	else if (die <= 485)
 	{
	    sprintf(suffix, "fleetness");
	    obj->perm_spell = ITEMA_HASTE;
	    obj_improve_stat(obj, APPLY_MOVE, dice(3, 200));
	    obj->value[6] = 5;
    	}
	else if (die <= 488)
 	{
	    sprintf(suffix, "resistance");
	    obj->perm_spell = ITEMA_PROTECTION_EVIL;
	    SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	    obj_improve_stat(obj, APPLY_HIT, dice(3, 200));
	    obj_improve_stat(obj, APPLY_SAVES, 0-dice(1, 3));
    	}
	else if (die <= 490)
 	{
	    sprintf(suffix, "godshome");
	    obj_improve_stat(obj, APPLY_HIT, dice(3, 200));
	    obj_improve_stat(obj, APPLY_MOVE, dice(3, 200));
	    obj_improve_stat(obj, APPLY_AC, 0 - dice(2, 20));
	    obj_improve_stat(obj, APPLY_STR, dice(1, 3));
	    obj_improve_stat(obj, APPLY_DEX, dice(1, 3));
	    obj_improve_stat(obj, APPLY_CON, dice(1, 3));
	    obj_improve_stat(obj, APPLY_CHA, dice(1, 3));
	    obj_improve_stat(obj, APPLY_WIS, dice(1, 3));
	    obj_improve_stat(obj, APPLY_INT, dice(1, 3));
	    obj->perm_spell = ITEMA_SANCTUARY;
	    SET_BIT(obj->extra_flags, ITEM_GLOW);
	    obj->value[1]++;
	    obj->value[2]++;
	}
	else if (die <= 492)
 	{
	    sprintf(suffix, "element storm");
	    SET_BIT(obj->value[4], WEAPON_FLAMING);	
	    SET_BIT(obj->value[4], WEAPON_FROST);	
	    SET_BIT(obj->value[4], WEAPON_POISON);	
	    SET_BIT(obj->value[4], WEAPON_SHOCKING);	
	    obj->value[1]++;
	    obj->value[2]++;
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	}
	else if (die <= 494)
 	{
	    sprintf(suffix, "cleaving");
	    obj->value[1] = 10;	    
	    obj->value[2] = 10;	    
	    SET_BIT(obj->value[4], WEAPON_FLAMING);	
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	    SET_BIT(obj->value[4], WEAPON_SHARP);
	    SET_BIT(obj->value[4], WEAPON_VORPAL);
	    obj->perm_spell = ITEMA_HASTE;
	    improve_damroll(obj, dice(6, 6) );
	    improve_hitroll(obj, dice(6, 6) );
	    obj->value[6] = 4;
	    obj->value[7] = 10;
	}
	else if (die <= 497)
 	{
	    sprintf(suffix, "divine aura");
	    obj->value[1] = 9;
	    obj->value[2] = 9;
	    obj->perm_spell = ITEMA_SANCTUARY;
	    improve_ac(obj, -200);
	    obj->value[7] = 10;
	    obj_improve_stat(obj, APPLY_DEX, dice(2, 5));
    	}
	else if (die <= 500)
 	{
	    sprintf(suffix, "dark hearts");
	    obj->value[1] = 11;
	    obj->value[2] = 11;
	    SET_BIT(obj->value[4], WEAPON_FLAMING);	
	    SET_BIT(obj->extra_flags, ITEM_FLAMING);
	    SET_BIT(obj->value[4], WEAPON_SHARP);
	    SET_BIT(obj->value[4], WEAPON_VORPAL);
	    SET_BIT(obj->value[4], WEAPON_FROST);
	    obj->perm_spell = ITEMA_HASTE;
	    improve_damroll(obj, dice(6, 10) );
	    improve_hitroll(obj, dice(6, 10) );
	    obj->value[6] = 3;
	    obj->value[7] = 10;
 	}

	obj->level = die/5;
    }

    if (weaptype[0] == '\0' || prefix[0] == '\0')
    {
	bug("Create random weapon - not working :)", 0);
	return NULL;
    }

    /* Now string the names together... now we have all the 3 parts */
    if (suffix[0] == '\0')
    {
	sprintf(totalname, "a%s %s %s",
	    is_vowel(prefix[0]) ? "n" : "",
	    prefix, weaptype);
	WRITE_STR(obj->short_descr, totalname);

	sprintf(totalname, "A%s %s %s lies here on the floor.",
	    is_vowel(prefix[0]) ? "n" : "",
	    prefix, weaptype);
	WRITE_STR(obj->description, totalname);

	sprintf(totalname, "%s %s",
	    prefix, weaptype);
	WRITE_STR(obj->name, totalname);
    }
    else
    {
	sprintf(totalname, "a%s %s %s of %s",
	    is_vowel(prefix[0]) ? "n" : "",
	    prefix, weaptype, suffix);
	WRITE_STR(obj->short_descr, totalname);

	sprintf(totalname, "A%s %s %s lies here on the floor.",
	    is_vowel(prefix[0]) ? "n" : "",
	    prefix, weaptype);
	WRITE_STR(obj->description, totalname);

	sprintf(totalname, "%s %s %s",
	    prefix, weaptype, suffix);
	WRITE_STR(obj->name, totalname);
    }

    return obj;

}

void do_treasure(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj = NULL;

    if (argument[0] == '\0')
    {
	stc("Syntax : treasure [weapon ring bow quiver arrow random]\n\r", ch);
	return;
    }

    if (!str_prefix(argument, "weapon"))
	obj = create_random_weapon(0);
    else if (!str_prefix(argument, "ring"))
	obj = create_random_ring(0);
    else if (!str_prefix(argument, "quiver"))
	obj = create_random_quiver(0);
    else if (!str_prefix(argument, "bow"))
	obj = create_random_bow(0);
    else if (!str_prefix(argument, "arrow"))
	obj = create_random_arrow(0);
    else if (!str_prefix(argument, "random"))
	obj = create_random_treasure(0);
    else
    {
	do_treasure(ch, "");
	return;
    }

    if (obj == NULL)
    {
        stc("You have created a NULLITEM.  Well done.\n\r", ch);
        return;
    }

    obj_to_char(obj, ch);
    stcf(ch, "You have randomly created %s.\n\r",
        obj->short_descr);
    act("$n has randomly created $p.", ch, obj, NULL, TO_ROOM);

    return;
}

OBJ_DATA * create_random_treasure( int something )
{
    int die = dice(1, 100);

    if (die <= 20) return create_random_weapon(0);
    else if (die <= 40) return create_random_ring(0);
    else if (die <= 60) return create_random_quiver(0);
    else if (die <= 80) return create_random_arrow(0);
    else if (die <= 100) return create_random_bow(0);

    return NULL;
}

