ifeq ($(shell [ ! -r ../../Make.Rules ] && echo 1),)
	include ../../Make.Rules
endif

CFLAGS   += -I.. -I../../include -Wall -W
LIBFLAGS := -lm -L../../lib -L../../localhash -lhashish_s -llocalhash

all: hashstr

hashstr: hashstr.c
	$(CC) $(CFLAGS) hashstr.c $(LIBFLAGS) -o $@

clean:
	$(RM) -f $(OBJ) hashstr

distclean:
	@true

install:
	@true

