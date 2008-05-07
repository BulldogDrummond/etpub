SRCS=$(wildcard *.c)
OBJECTS=$(subst .c,.cg,$(SRCS))
WIN_OBJECTS=$(subst .cg,.wcg,$(OBJECTS))

GAME_OBJECTS=../game/bg_animation.cg ../game/bg_animgroup.cg ../game/bg_character.cg ../game/bg_classes.cg ../game/bg_misc.cg ../game/bg_pmove.cg ../game/bg_slidemove.cg ../game/bg_sscript.cg ../game/bg_stats.cg ../game/bg_tracemap.cg ../game/q_math.cg ../game/q_shared.cg
WIN_GAME_OBJECTS=$(subst .cg,.wcg,$(GAME_OBJECTS))

UI_OBJECTS=../ui/ui_shared.cg
WIN_UI_OBJECTS=$(subst .cg,.wcg,$(UI_OBJECTS))

ALL_OBJECTS=$(OBJECTS) $(GAME_OBJECTS) $(UI_OBJECTS)
WIN_ALL_OBJECTS=$(WIN_OBJECTS) $(WIN_GAME_OBJECTS) $(WIN_UI_OBJECTS)

CFLAGS=$(CGAME_CFLAGS)

LIBS=cgame.mp.i386.so cgame_mp_x86.dll

all: depend $(LIBS)

cgame.mp.i386.so: $(ALL_OBJECTS)
	$(CC) $(LDFLAGS) $(ALL_OBJECTS) -o $@
	#${STRIP} $@

cgame_mp_x86.dll: $(WIN_ALL_OBJECTS)
	$(MINGW) $(WIN_LDFLAGS) $(WIN_ALL_OBJECTS) /usr/i586-mingw32msvc/lib/libpsapi.a -o $@
	#${WIN_STRIP} $@

depend: $(SRCS)
	touch Makefile.depend
	$(DEPEND) $(INCLUDES) -fMakefile.depend -o.cg $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wcg $(SRCS) $(DEPEND_REDIR)

%.cg: %.c
	$(CC) -c $(CGAME_CFLAGS) $< -o $@

%.wcg: %.c
	$(MINGW) -c $(CGAME_WIN_CFLAGS) $< -o $@

clean:
	-rm $(LIBS) *.cg *.wcg Makefile.depend*

-include Makefile.depend

