/*************************************************************
*     Mob Progs - Module Version 2.00 (C) Chris Mellish      *
* +--------------------------------------------------------+ *
*     Interpreter	- written by Chris Mellish           *
*     Misc Functions    - written by Chris Mellish           *
*                         and his pet marrigold Adam Powell  *
*       Trigger Code    - written by Chris Mellish	     *
*             &             and Adam Powell                  *
*     Default Functions                                      *
*************************************************************/

/*************************************************************
* FN functions which the interpreter can call upon	     *
*************************************************************/

typedef void MP_FUN	args( ( PROG_DATA *prog, char *argument, char *ret) );

#define NUM_FNS			20

struct mp_fn_type
{
    char name[40];
    MP_FUN *proc;
};


#define DECLARE_MP_FUN( fun )	MP_FUN fun


DECLARE_MP_FUN( mp_str_prefix );
DECLARE_MP_FUN( mp_unfreeze   );
DECLARE_MP_FUN( mp_running    );
DECLARE_MP_FUN( mp_spawn      );
DECLARE_MP_FUN( mp_pc_marrying);
DECLARE_MP_FUN( mp_pass_var   );
DECLARE_MP_FUN( mp_male_female);
DECLARE_MP_FUN( mp_mob_here   );
DECLARE_MP_FUN( mp_str_cmp    );
DECLARE_MP_FUN( mp_random     );
DECLARE_MP_FUN( mp_dice       );
DECLARE_MP_FUN( mp_multi_check);
DECLARE_MP_FUN( mp_arg_check  );
DECLARE_MP_FUN( mp_curr_stance);
DECLARE_MP_FUN( mp_curr_hp    );
DECLARE_MP_FUN( mp_eval	      );
DECLARE_MP_FUN( mp_claim_trigger);
DECLARE_MP_FUN( mp_release_trigger);

const struct mp_fn_type mp_fn[NUM_FNS] =
{
    { "str_prefix"	, mp_str_prefix },
    { "unfreeze"	, mp_unfreeze   },
    { "running"		, mp_running    },
    { "spawn"		, mp_spawn      },
    { "pass_var"	, mp_pass_var   },
    { "mob_here"	, mp_mob_here   },
    { "pc_marrying"	, mp_pc_marrying},
    { "male_female"	, mp_male_female},
    { "str_cmp"		, mp_str_cmp	},
    { "random"		, mp_random	},
    { "multi_check"	, mp_multi_check},
    { "arg_check"	, mp_arg_check	},
    { "curr_stance"	, mp_curr_stance},
    { "curr_hp"		, mp_curr_hp	},
    { "eval"		, mp_eval	},
    { "claim_trigger"	, mp_claim_trigger},
    { "release_trigger"	, mp_release_trigger},
    { "dice"		, mp_dice	},
    { ""		, NULL		}
};
