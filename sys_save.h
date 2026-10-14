/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
#define NUM_GLOBAL_VARIABLES	10

void	save_sysvars	args( ( ) );


/* flag tables */
extern	const	struct	global_variable_type glob_var[NUM_GLOBAL_VARIABLES];

/*
 * new global system settings
 */
 
struct global_variable_type {
    int value; 
    int min_val; 
    int max_val; 
    char name[20]; 
}; 

const struct global_variable_type glob_var [NUM_GLOBAL_VARIABLES] =
{
    {
	0, 0, 0,
	"Min_Save_Level"
    }
}
