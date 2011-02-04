# $Id$

ifeq ($(shell test \! -f Make.Rules || echo yes),yes)
		include Make.Rules
endif

SUBDIRS := include lib localhash tests analysis doc man
.PHONY: all clean distclean install release releasebz2 $(SUBDIRS)

all: Make.Rules
	@for dir in $(SUBDIRS); do \
		 echo "### Entering $$dir" && cd $$dir && $(MAKE) && cd ..; \
	done

clean: Make.Rules
	@for dir in $(SUBDIRS); do \
		echo "### Entering $$dir" && cd $$dir && $(MAKE) clean && cd ..; \
	done

test:
	cd tests && $(MAKE) test

distclean: clean
	rm -rf Make.Rules config.h config.log

install: Make.Rules
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) install && cd ..; \
	done

uninstall: Make.Rules
	@for dir in $(SUBDIRS); do \
		echo "### Entering $$dir" && cd $$dir && $(MAKE) uninstall && cd ..; \
	done

config.h: Make.Rules

Make.Rules: configure
	@if [ ! -f Make.Rules ] ; then                   \
	echo "No Make.Rules file present" ;              \
	echo "Hint: call ./configure script" ;           \
	echo "./configure --help for more information" ; \
  exit 1 ; \
	fi

config.h: 
	@bash configure


show: Make.Rules
	@echo "PACKAGE_NAME: $(PACKAGE_NAME)"
	@echo "MAJOR_REL:    $(MAJOR_REL)"
	@echo "MINOR_REL:    $(MINOR_REL)"
	@echo
	@echo "AR:           $(AR)"
	@echo "LD:           $(LD)"
	@echo "RANLIB:       $(RANLIB)"
	@echo "MKDIR:        $(MKDIR)"
	@echo "INSTALL:      $(INSTALL)"
	@echo "CC:           $(CC)"
	@echo "CFLAGS:       $(CFLAGS)"
	@echo "LIBGDFLAGS:   $(LIBGDFLAGS)"
	@echo "LDFLAGS:      $(LDFLAGS)"
	@echo "DESTDIR:      $(DESTDIR)"
	@echo "prefix:       $(prefix)"
	@echo "bindir:       $(bindir)"
	@echo "libdir:       $(libdir)"
	@echo "TOPDIR:       $(TOPDIR)"
	@echo "includedir:   $(includedir)"
													  
cscope:
	cscope -b -q -R -Iinclude -slib -stest
																 
																  
$(SUBDIRS):
	cd $@ && $(MAKE)
																			 


DISTNAME=$(PACKAGE_NAME)

release:
	@if [ ! -f Make.Rules ]; then echo $(MAKE) Make.Rules first ;exit 1 ;fi
	@if [ ! -L ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL) ]; then \
		echo generating ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL) link ; \
		ln -sf $(DISTNAME) ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL) ; \
		echo to ../$(DISTNAME) . ; fi
	@diff ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL)/Make.Rules Make.Rules
	$(MAKE) distclean
	cd .. ; tar zvfc $(DISTNAME)-$(MAJOR_REL).$(MINOR_REL).tar.gz \
		--exclude .svn  --exclude '.#*' \
		$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL)/*

releasebz2:
	@if [ ! -f Make.Rules ]; then echo $(MAKE) Make.Rules first ;exit 1 ;fi
	@if [ ! -L ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL) ]; then \
		echo generating ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL) link ; \
		ln -sf $(DISTNAME) ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL) ; \
		echo to ../$(DISTNAME) . ; fi
	@diff ../$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL)/Make.Rules Make.Rules
	$(MAKE) distclean
	cd .. ; tar jvfc $(DISTNAME)-$(MAJOR_REL).$(MINOR_REL).tar.bz2 \
		--exclude .svn  --exclude '.#*' \
		$(DISTNAME)-$(MAJOR_REL).$(MINOR_REL)/*

-include Make.Rules
