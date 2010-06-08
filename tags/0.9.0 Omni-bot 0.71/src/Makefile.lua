include Makefile

ALL_LUA_OBJ=$(CORE_O) $(LIB_O)
ETPUB_LUA_OBJ=$(subst .o,.wo,$(ALL_LUA_OBJ)) $(subst .o,.to,$(ALL_LUA_OBJ))

etpub: $(ETPUB_LUA_OBJ)

%.wo: %.c
	$(MINGW) -c $(WIN_CFLAGS) $< -o $@

%.to: %.c
	$(CC) -c $(CFLAGS) $< -o $@
