# $Id$

ifeq ($(shell [ ! -r ../Make.Rules ] && echo 1),)
	include ../Make.Rules
endif


all: libhashish.pod
	pod2man --section=7 --center libhashish --date 0.1 libhashish.pod > libhashish.7
	pod2man --section=7 --center libhashish --date 0.1 hi_bloom_mk.pod > hi_bloom_mk.7

display: all
	groff -man -Tascii libhashish.7
	groff -man -Tascii hi_bloom_mk.7

ps: all
	groff -man -Tps libhashish.7 > libhashish.ps
	groff -man -Tps hi_bloom_mk.7 > hi_bloom_mk.ps

pdf: ps
	ps2pdf libhashish.ps
	ps2pdf hi_bloom_mk.ps

clean:
	rm -rf *.7 *.pdf *.ps *.tmp

distclean: clean
	@true

install:
	@true

uninstall:
	@true

.PHONY: all clean install uninstall
