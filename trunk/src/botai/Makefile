SRCS=$(wildcard *.c) 
OBJECTS=$(subst .c,.bai, $(SRCS)) 
CFLAGS=$(GAME_CFLAGS)
ALL_OBJECTS=$(OBJECTS)

all: depend $(OBJECTS) 

depend: $(SRCS)
	touch Makefile.depend
	$(DEPEND) $(INCLUDES) -fMakefile.depend -o.bai $(SRCS) $(DEPEND_REDIR)
	
%.bai: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-rm *.bai Makefile.depend*

-include Makefile.depend


