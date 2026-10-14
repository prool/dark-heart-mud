CC      = gcc
PROF    = -O -g
#enable this if you want to use gprof
#PROF	= -O -g -pg
NOCRYPT =
C_FLAGS =  -Wall $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)
PROG	= btl

SRC = *.c
INC = *.h

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o db.o db2.o effects.o fight.o handler.o interp.o note.o lookup.o magic.o recycle.o save.o skills.o tables.o update.o psionics.o act_dark.o darkmag.o core.o act_evil.o hero.o stat_sav.o prog.o voodoo.o misc.o heart.o olc.o system.o combat.o ansi.o vampire.o trade.o runes.o treasure.o mage.o bestiary.o pandora.o

all: $(O_FILES)
	rm -f ${PROG}
	$(CC) $(L_FLAGS) -o ${PROG} $(O_FILES)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean: 
	rm -f core *.o ${PROG} nohup.out
distclean: clean 
	rm -f *~
checkin:
	ci -l $(SRC) $(INC)
diff:
	-rcsdiff -u $(SRC) $(INC) > diff.new
	gzip diff.new
patch:
	gunzip -c -d diff.new.gz | patch
upload:
	echo insert an upload script here
