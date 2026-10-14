DECLARE_DO_FUNC( do_say		);
DECLARE_DO_FUNC( do_listhouse   );

void do_listhouse(CHAR_DATA *ch, char *argument)
{
    if (rb_mob == NULL || rb_mob->in_room == NULL
    	|| rb_mob->in_room != ch->in_room)
    {
        stc("Billy the Brickie doesn't seem to be here.\n\r", ch);
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
    stc("following:  #7house#n, #7extension#n, #7door#n, #7wizlocked#n, #7fountain#n\n\r", ch;
    do_say(ch, "Thankyou.");
}

void do_buyhouse(CHAR_DATA *ch, char *argument)
{
    int type = 0;
    char arg[MAX_INPUT_LENGTH];

    if (rb_mob == NULL || rb_mob->in_room == NULL
    	|| rb_mob->in_room != ch->in_room)
    {
        stc("Billy the Brickie doesn't seem to be here.\n\r", ch);
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

    stc("That option, while recognised, has yet to be coded\n\r", ch);
}