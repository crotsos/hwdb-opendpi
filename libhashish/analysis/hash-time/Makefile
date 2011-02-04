ifeq ($(shell [ ! -r ../../Make.Rules ] && echo 1),)
	include ../../Make.Rules
endif

CIN      := $(wildcard *.c)
OBJ      := $(CIN:%.c=%.o)
CFLAGS   += -I.. -I../../include -Wall -W
LIBFLAGS := -lm -L../../lib -L../../localhash -lhashish_s -llocalhash

export

.PHONY: all clean bench

all: benchtool

benchtool: $(OBJ)
	$(CC) $(CFLAGS) benchtool.o $(LIBFLAGS) -o $@

clean:
	@echo "### cleaning"; \
	$(RM) -f $(OBJ) benchtool bench-*.data bench.pdf

distclean:
	@echo "### distclean"
	@true

install:
	@true

