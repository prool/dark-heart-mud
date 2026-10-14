/*****************************************************************************
*									     *
*   AUTOMATIC MONSTER GENERATOR!!!!!  By          	                     *
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

/* Moria eat your heart out... this is the biggest and best automatic monster
generator in the world.  And its only compatible with HELL(TM) code */

CHAR_DATA * create_blobling( )
{
    CHAR_DATA *mob;

    mob = create_mobile(get_mob_index(29));
    mob->mob_spec = 0;
    mob->affected_by = 0;
    mob->flag2 = 0;
    mob->flag3 = 0;
    mob->flag4 = 0;
    mob->flag5 = 0;
    mob->flag6 = 0;

    return mob;
}

