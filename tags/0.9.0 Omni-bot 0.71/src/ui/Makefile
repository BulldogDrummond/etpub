SRCS=$(wildcard *.c)
OBJECTS=$(subst .c,.ui,$(SRCS))
GAME_OBJECTS=../game/bg_campaign.ui ../game/bg_classes.ui ../game/bg_misc.ui ../game/q_math.ui ../game/q_shared.ui
ALL_OBJECTS=$(OBJECTS) $(GAME_OBJECTS)
WIN_GAME_OBJECTS=$(subst .ui,.wui,$(GAME_OBJECTS))
WIN_OBJECTS=$(subst .c,.wui,$(SRCS))
WIN_ALL_OBJECTS=$(WIN_OBJECTS) $(WIN_GAME_OBJECTS)

CGAME_SRCS=ui_shared.c
CGAME_OBJECTS=$(subst .c,.cg,$(CGAME_SRCS))
WIN_CGAME_OBJECTS=$(subst .cg,.wcg,$(CGAME_OBJECTS))
CGAME=$(CGAME_OBJECTS) $(WIN_CGAME_OBJECTS)

CFLAGS=$(UI_CFLAGS)

LIBS=ui.mp.i386.so ui_mp_x86.dll

all: depend $(LIBS) $(CGAME)

ui.mp.i386.so: $(ALL_OBJECTS)
	$(CC) $(LDFLAGS) $(ALL_OBJECTS) -o $@
	#${STRIP} $@

ui_mp_x86.dll: $(WIN_ALL_OBJECTS)
	$(MINGW) $(UI_WIN_LDFLAGS) $(WIN_ALL_OBJECTS) -o $@
	#${WIN_STRIP} $@

%.ui: %.c
	$(CC) -c $(CGAME_CFLAGS) $< -o $@

%.wui: %.c
	$(MINGW) -c $(UI_WIN_CFLAGS) $< -o $@

%.cg: %.c
	$(CC) -c $(CGAME_CFLAGS) $< -o $@

%.wcg: %.c
	$(MINGW) -c $(CGAME_WIN_CFLAGS) $< -o $@

depend: $(SRCS)
	touch Makefile.depend
	$(DEPEND) $(INCLUDES) -fMakefile.depend -o.o.ui $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wui $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.cg $(CGAME_SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wcg $(CGAME_SRCS) $(DEPEND_REDIR)

clean:
	-rm $(LIBS) *.ui *.wui *.cg *.wcg Makefile.depend*

-include Makefile.depend

