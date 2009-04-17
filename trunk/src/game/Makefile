SRCS=$(wildcard *.c) ../sha-1/sha1.c
SRCSCPP=$(wildcard *.cpp) ../../Omnibot/Common/BotLoadLibrary.cpp
OBJECTS=$(subst .c,.o,$(SRCS)) $(subst .cpp,.oxx,$(SRCSCPP))
WIN_OBJECTS=$(subst .c,.wo,$(SRCS)) $(subst .cpp,.woxx,$(SRCSCPP))
ALL_OBJECTS=$(OBJECTS) $(WIN_OBJECTS)

UI_OBJECTS=../game/bg_campaign.ui ../game/bg_classes.ui ../game/bg_misc.ui ../game/q_math.ui ../game/q_shared.ui

WIN_UI_OBJECTS=$(subst .ui,.wui,$(UI_OBJECTS))
UI=$(UI_OBJECTS) $(WIN_UI_OBJECTS)

CGAME_OBJECTS=../game/bg_animation.cg ../game/bg_animgroup.cg ../game/bg_character.cg ../game/bg_classes.cg ../game/bg_misc.cg ../game/bg_pmove.cg ../game/bg_slidemove.cg ../game/bg_sscript.cg ../game/bg_stats.cg ../game/bg_tracemap.cg ../game/q_math.cg ../game/q_shared.cg

WIN_CGAME_OBJECTS=$(subst .cg,.wcg,$(CGAME_OBJECTS))
CGAME=$(CGAME_OBJECTS) $(WIN_CGAME_OBJECTS)

CFLAGS=$(GAME_CFLAGS)
WIN_CFLAGS=$(GAME_WIN_CFLAGS)

export DEPEND DEPEND_REDIR INCLUDES GAME_CFLAGS

all: qagame.mp.i386.so qagame_mp_x86.dll $(UI) $(CGAME)

qagame.mp.i386.so: botai lua depend $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(wildcard ../botai/*.bai) $(wildcard ../$(LUADIR)/src/*.to) -o $@
	#${STRIP} $@

qagame_mp_x86.dll: botai lua depend $(WIN_OBJECTS)
	$(MINGWXX) $(WIN_LDFLAGS) $(WIN_OBJECTS) $(wildcard ../botai/*.wbai) $(wildcard ../$(LUADIR)/src/*.wo) -lws2_32 -o $@
	#${WIN_STRIP} $@

botai: 
	$(MAKE) -C ../botai

lua:
	#$(MAKE) -C ../$(LUADIR) linux
	cp ../Makefile.lua ../$(LUADIR)/src
	$(MAKE) -C ../$(LUADIR)/src -f Makefile.lua etpub
	rm ../$(LUADIR)/src/Makefile.lua

depend: $(SRCS)
	touch Makefile.depend
	$(DEPEND) $(INCLUDES) -fMakefile.depend -o.o $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.o $(SRCSCPP) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wo $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wo $(SRCSCPP) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.ui $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wui $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.cg $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -a -o.wcg $(SRCS) $(DEPEND_REDIR)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.oxx: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.wo: %.c
	$(MINGW) -c $(WIN_CFLAGS) $< -o $@

%.woxx: %.cpp
	$(MINGWXX) -c $(WIN_CXXFLAGS) $< -o  $@

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
	$(MAKE) -C ../$(LUADIR) clean
	-rm qagame_mp_x86.dll qagame.mp.i386.so *.o *.oxx *.wo *.woxx *.ui *.wui *.cg *.wcg Makefile.depend*
	-rm ../../Omnibot/Common/BotLoadLibrary.*oxx
	-rm ../sha-1/sha1.*o
	-rm ../$(LUADIR)/src/*.?o

-include Makefile.depend

