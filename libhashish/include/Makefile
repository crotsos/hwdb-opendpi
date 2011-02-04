# $Id: Makefile 1 2007-05-09 13:17:04Z hgndgtl $

ifeq ($(shell [ ! -r ../Make.Rules ] && echo 1),)
	include ../Make.Rules
endif

.PHONY: all clean install uninstall

all:
	@true

clean:
	@true

distclean: clean
	@true

install:
	install -m 0644 libhashish.h $(DESTDIR)$(includedir)/

uninstall:
	@true
	rm -rf $(DESTDIR)$(includedir)/libhashish.h
