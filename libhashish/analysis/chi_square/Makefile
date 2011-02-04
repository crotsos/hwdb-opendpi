# $Id: Makefile 20 2008-01-23 23:46:02Z hgndgtl $

ifeq ($(shell [ ! -r ../../Make.Rules ] && echo 1),)
	include ../../Make.Rules
endif

CIN      := $(wildcard *.c)
OBJ      := $(CIN:%.c=%.o)
CFLAGS   += -I.. -I../../include -Wall -W
LIBFLAGS := -lm -L../../lib -L../../localhash -lhashish_s -llocalhash

export

.PHONY: all clean chi-square

all: chi_square

chi_square: $(OBJ)
	$(CC) $(CFLAGS) chi_square.o $(LIBFLAGS) -o $@

clean:
	@echo "### cleaning"; \
	$(RM) -f $(OBJ) chi_square *.png *.pdf *.data core*

distclean:
	@echo "### distclean"
	@true

install:
	@true

