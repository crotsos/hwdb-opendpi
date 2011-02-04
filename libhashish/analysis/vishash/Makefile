ifeq ($(shell [ ! -r ../../Make.Rules ] && echo 1),)
	include ../../Make.Rules
endif

CFLAGS   += -I.. -I../../include -Wall -W
LIBFLAGS := -lm -L../../lib -L../../localhash -lhashish_s -llocalhash -lgd

ifneq (,$(findstring gd,$(LIBGDFLAGS)))
all: vishash

vishash: vishash.c
	$(CC) $(CFLAGS) vishash.c $(LIBFLAGS) -o $@
else
all: 
endif

clean:
	$(RM) -f $(OBJ) vishash
	$(RM) -f *.png

distclean:
	@true

install:
	@true

