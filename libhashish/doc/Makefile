# $Id$

ifeq ($(shell [ ! -r ../Make.Rules ] && echo 1),)
	include ../Make.Rules
endif

.PHONY: all clean install uninstall

all:
	@true

gendoc:
	mkdir html
	doxygen Doxyfile

distclean: clean
	$(RM) -f html/*

clean:
	@true
	rm -rf html

install:
	@true

uninstall:
	@true
