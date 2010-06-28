SRCS=$(wildcard *.c) 
OBJECTS=$(subst .c,.bai, $(SRCS)) 
WIN_OBJECTS=$(subst .c,.wbai, $(SRCS)) 
CFLAGS=$(GAME_CFLAGS)
ALL_OBJECTS=$(OBJECTS) $(WIN_OBJECTS)

all: depend $(ALL_OBJECTS) 

depend: $(SRCS)
	touch Makefile.depend
	$(DEPEND) $(INCLUDES) -fMakefile.depend -o.bai $(SRCS) $(DEPEND_REDIR)
	$(DEPEND) $(INCLUDES) -fMakefile.depend -o.wbai $(SRCS) $(DEPEND_REDIR)
	
%.bai: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.wbai: %.c
	$(MINGW) -c $(WIN_CFLAGS) $< -o $@

clean:
	-rm *.bai *.wbai Makefile.depend*

-include Makefile.depend


