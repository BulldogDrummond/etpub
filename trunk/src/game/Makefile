SRCS=$(wildcard *.c)
SRCSCPP=$(wildcard *.cpp) ../../Omnibot/Common/BotLoadLibrary.cpp
OBJECTS=$(subst .c,.o,$(SRCS)) $(subst .cpp,.oxx,$(SRCSCPP))
ALL_OBJECTS=$(OBJECTS)

UI_OBJECTS=../game/bg_campaign.ui ../game/bg_classes.ui ../game/bg_misc.ui ../game/q_math.ui ../game/q_shared.ui

WIN_UI_OBJECTS=$(subst .ui,.wui,$(UI_OBJECTS))
UI=$(UI_OBJECTS) $(WIN_UI_OBJECTS)

CGAME_OBJECTS=../game/bg_animation.cg ../game/bg_animgroup.cg ../game/bg_character.cg ../game/bg_classes.cg ../game/bg_misc.cg ../game/bg_pmove.cg ../game/bg_slidemove.cg ../game/bg_sscript.cg ../game/bg_stats.cg ../game/bg_tracemap.cg ../game/q_math.cg ../game/q_shared.cg

WIN_CGAME_OBJECTS=$(subst .cg,.wcg,$(CGAME_OBJECTS))
CGAME=$(CGAME_OBJECTS) $(WIN_CGAME_OBJECTS)

CFLAGS=$(GAME_CFLAGS)

export DEPEND DEPEND_REDIR INCLUDES GAME_CFLAGS

all: qagame.mp.i386.so $(UI) $(CGAME)

qagame.mp.i386.so: botai depend $(ALL_OBJECTS)
	$(CXX) $(LDFLAGS) $(ALL_OBJECTS) $(wildcard ../botai/*.bai) -o $@
	#${STRIP} $@

botai: 
	$(MAKE) -C ../botai

depend: $(SRCS)
	touch Makefile.depend
	$(DEPEND) $(INCLUDES) -fMakefile.depend $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a $(SRCSCPP) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.ui $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wui $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.cg $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wcg $(SRCS) $(DEPEND_REDIR)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	
%.oxx: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.ui: %.c
	$(CC) -c $(UI_CFLAGS) $< -o $@

%.wui: %.c
	$(MINGW) -c $(UI_WIN_CFLAGS) $< -o $@

%.cg: %.c
	$(CC) -c $(CGAME_CFLAGS) $< -o $@

%.wcg: %.c
	$(MINGW) -c $(CGAME_WIN_CFLAGS) $< -o $@
	
clean:
	$(MAKE) -C ../botai clean
	-rm qagame.mp.i386.so *.o *.oxx *.ui *.wui *.cg *.wcg Makefile.depend*
	-rm ../../Omnibot/Common/BotLoadLibrary.oxx

-include Makefile.depend

