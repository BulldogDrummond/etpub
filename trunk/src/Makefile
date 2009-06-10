OPT=-O3
SERVER_OBJECTS=qagame.mp.i386.so qagame_mp_x86.dll
CLIENT=etpub.pk3
CLIENT_OBJECTS=cgame.mp.i386.so cgame_mp_x86.dll ui.mp.i386.so ui_mp_x86.dll
CC=gcc-3.3
CXX=g++-3.3
MINGW=i586-mingw32msvc-gcc
MINGWXX=i586-mingw32msvc-g++
GAME_FLAGS=-march=pentium4 -DGAMEDLL -DUSE_MDXFILE -DNO_BOT_SUPPORT -DTHREADS -DLUA_SUPPORT
CGAME_FLAGS=-march=i686 -D_M_IX86=400 -DCGAMEDLL -DUSE_MDXFILE
UI_FLAGS=-march=i686 -D_M_IX86=400 -DUIDLL
DEPEND=-makedepend
DEPEND_REDIR=2> /dev/null
LUADIR=lua-5.1.4
INCLUDES=-I. -I../../Omnibot/ET -I../../Omnibot/Common -I../$(LUADIR)/src -I../sha-1
BASE_CFLAGS=$(INCLUDES) -pipe -Wall -fmessage-length=0 -DNDEBUG $(OPT) -Winline -ffast-math -fomit-frame-pointer -finline-functions -fschedule-insns2 -m32 -fno-strict-aliasing
#CFLAGS=$(BASE_CFLAGS) -fPIC -fno-function-cse -fvisibility=hidden
CFLAGS=$(BASE_CFLAGS) -fPIC
CXXFLAGS=-fpermissive $(CFLAGS)
WIN_CXXFLAGS=-fpermissive $(BASE_CFLAGS)

GAME_CFLAGS=$(CFLAGS) $(GAME_FLAGS)
GAME_WIN_CFLAGS=$(BASE_CFLAGS) $(GAME_FLAGS)
CGAME_CFLAGS=$(CFLAGS) $(CGAME_FLAGS)
CGAME_WIN_CFLAGS=$(BASE_CFLAGS) $(CGAME_FLAGS) 
UI_CFLAGS=$(CFLAGS) $(UI_FLAGS)
UI_WIN_CFLAGS=$(BASE_CFLAGS) $(UI_FLAGS)

BASE_LDFLAGS=-shared -m32 -s
LDFLAGS=$(BASE_LDFLAGS) -rdynamic
WIN_LDFLAGS=$(BASE_LDFLAGS) -static-libgcc 
UI_WIN_LDFLAGS=$(WIN_LDFLAGS) -lws2_32

STRIP=strip
WIN_STRIP=i586-mingw32msvc-strip

export CC CXX MINGW MINGWXX WIN_EXT OPT DEPEND INCLUDES GAME_CFLAGS CGAME_CFLAGS CGAME_WIN_CFLAGS UI_CFLAGS UI_WIN_CFLAGS CXXFLAGS LDFLAGS WIN_LDFLAGS UI_WIN_LDFLAGS DEPEND_REDIR STRIP WIN_STRIP LUADIR GAME_WIN_CFLAGS WIN_CXXFLAGS

all: tmp $(SERVER_OBJECTS) $(CLIENT)

tmp:
	-mkdir ../tmp

$(SERVER_OBJECTS): 
	$(MAKE) -C game

$(CLIENT): $(CLIENT_OBJECTS)
	cp -L -r --copy-contents ../bundle/* ../tmp
	-find ../tmp -name '.svn' -exec rm -rf '{}' \;
	cd ../tmp; zip -r $(CLIENT) *; mv $(CLIENT) ../src
	-rm -rf ../tmp
	-$(MAKE) proper

$(CLIENT_OBJECTS): build-cgame build-ui

build-ui: $(SERVER)
	$(MAKE) -C ui

build-cgame: build-ui
	$(MAKE) -C cgame

proper:
	ln -s etpub.pk3 etpubc-`python client_ver.py`.pk3

clean:
	$(MAKE) -C game clean
	$(MAKE) -C ui clean
	$(MAKE) -C cgame clean
	-rm -f etpub*.pk3
	-rm -rf ../tmp

