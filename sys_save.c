/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "sys_save.h"
 
/*
 * Local functions.
 */

void    fwrite_sysvars	args( ( FILE *fp ) );



/*
 * Save the system variables
 */
void save_sysvars( )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;


    sprintf( strsave, "sysvar.sav" );
    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
	bug( "Save_sysvars: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_sysvars( fp );
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    return;
}



/*
 * Write the char.
 */
void fwrite_sysvars( FILE *fp )
{
    return;
}

