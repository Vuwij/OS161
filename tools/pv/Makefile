#
# Files from which this is generated (inside directory `autoconf/make'):
#
#   package.mk          # package name and distribution details
#   vars.mk             # compilation, shell and linking variables
#   filelist.mk~        # lists of files
#   unreal.mk           # phony targets
#   modules.mk~         # module linking rules
#   rules.mk            # compilation rules
#   link.mk             # real top-level targets
#   depend.mk~          # dependencies
#
#

#
# Variables for Make.
#

srcdir = .

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
infodir = ${prefix}/share/info
mandir = ${prefix}/share/man
etcdir = /usr/local/etc
datadir = ${prefix}/share
sbindir = ${exec_prefix}/sbin



localedir = $(datadir)/locale
gnulocaledir = $(prefix)/share/locale

CATALOGS =  src/nls/de.mo src/nls/fr.mo src/nls/pl.mo src/nls/pt.mo
POFILES =  $(srcdir)/src/nls/de.po $(srcdir)/src/nls/fr.po $(srcdir)/src/nls/pl.po $(srcdir)/src/nls/pt.po
GMSGFMT = /usr/bin/msgfmt
MSGFMT = /usr/bin/msgfmt
XGETTEXT = /usr/bin/xgettext
MSGMERGE = msgmerge
CATOBJEXT = .mo
INSTOBJEXT = .mo


SHELL = /bin/sh
CC = gcc
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
UNINSTALL = rm -f

LDFLAGS = -r
LINKFLAGS = 
DEFS = -DHAVE_CONFIG_H -DLOCALEDIR=\"$(localedir)\"
CFLAGS = -O
CPPFLAGS =  -I$(srcdir)/src/include -Isrc/include $(DEFS)
LIBS = 

alltarg = pv

# EOF
#
# Package name, version, and distribution files.
#

package = pv
version = 1.6.0
PACKAGE = pv

distfiles =		\
$(srcdir)/README	\
$(srcdir)/autoconf	\
$(srcdir)/configure	\
$(srcdir)/doc		\
$(srcdir)/src		\
$(srcdir)/tests

# EOF
# Automatically generated file listings
#
# Creation time: Sun Mar 15 00:38:16 GMT 2015

allsrc = src/library/getopt.c \
src/library/gettext.c \
src/pv/signal.c \
src/pv/watchpid.c \
src/pv/cursor.c \
src/pv/file.c \
src/pv/display.c \
src/pv/loop.c \
src/pv/number.c \
src/pv/transfer.c \
src/pv/state.c \
src/main/version.c \
src/main/debug.c \
src/main/main.c \
src/main/options.c \
src/main/remote.c \
src/main/help.c

allobj = src/library/getopt.o \
src/library/gettext.o \
src/pv/signal.o \
src/pv/watchpid.o \
src/pv/cursor.o \
src/pv/file.o \
src/pv/display.o \
src/pv/loop.o \
src/pv/number.o \
src/pv/transfer.o \
src/pv/state.o \
src/main/version.o \
src/main/debug.o \
src/main/main.o \
src/main/options.o \
src/main/remote.o \
src/main/help.o \
src/library.o \
src/pv.o \
src/nls.o \
src/main.o

alldep = src/library/getopt.d \
src/library/gettext.d \
src/pv/signal.d \
src/pv/watchpid.d \
src/pv/cursor.d \
src/pv/file.d \
src/pv/display.d \
src/pv/loop.d \
src/pv/number.d \
src/pv/transfer.d \
src/pv/state.d \
src/main/version.d \
src/main/debug.d \
src/main/main.d \
src/main/options.d \
src/main/remote.d \
src/main/help.d

#
# Rules for all phony targets.
#

.PHONY: all help make dep depend test check \
  clean depclean indentclean distclean cvsclean svnclean \
  index manhtml indent update-po \
  doc dist release \
  install uninstall \
  rpm srpm

all: $(alltarg) $(CATALOGS)

help:
	@echo 'This Makefile has the following utility targets:'
	@echo
	@echo '  all             build all binary targets'
	@echo '  install         install compiled package and manual'
	@echo '  uninstall       uninstall the package'
	@echo '  check / test    run standardised tests on the compiled binary'
	@echo
	@echo 'Developer targets:'
	@echo
	@echo '  make            rebuild the Makefile (after adding new files)'
	@echo '  dep / depend    rebuild .d (dependency) files'
	@echo '  clean           remove .o (object) and .c~ (backup) files'
	@echo '  depclean        remove .d (dependency) files'
	@echo '  indentclean     remove files left over from "make indent"'
	@echo '  distclean       remove everything not distributed'
	@echo '  cvsclean        remove everything not in CVS/SVN'
	@echo
	@echo '  index           generate an HTML index of source code'
	@echo '  manhtml         output HTML man page to stdout'
	@echo '  indent          reformat all source files with "indent"'
	@echo '  update-po       update the .po files'
	@echo
	@echo '  dist            create a source tarball for distribution'
	@echo '  rpm             build a binary RPM (passes $$RPMFLAGS to RPM)'
	@echo '  srpm            build a source RPM (passes $$RPMFLAGS to RPM)'
	@echo '  release         dist+rpm+srpm'
	@echo

make:
	echo > $(srcdir)/autoconf/make/filelist.mk~
	echo > $(srcdir)/autoconf/make/modules.mk~
	cd $(srcdir); \
	bash autoconf/scripts/makemake.sh \
	     autoconf/make/filelist.mk~ \
	     autoconf/make/modules.mk~
	sh ./config.status

dep depend: $(alldep)
	echo '#' > $(srcdir)/autoconf/make/depend.mk~
	echo '# Dependencies.' >> $(srcdir)/autoconf/make/depend.mk~
	echo '#' >> $(srcdir)/autoconf/make/depend.mk~
	echo >> $(srcdir)/autoconf/make/depend.mk~
	cat $(alldep) >> $(srcdir)/autoconf/make/depend.mk~
	sh ./config.status

clean:
	rm -f $(allobj)

depclean:
	rm -f $(alldep)

indentclean:
	cd $(srcdir) && for FILE in $(allsrc); do rm -f ./$${FILE}~; done

update-po: $(srcdir)/src/nls/$(PACKAGE).pot
	catalogs='$(CATALOGS)'; \
	for cat in $$catalogs; do \
	  lang=$(srcdir)/`echo $$cat | sed 's/$(CATOBJEXT)$$//'`; \
	  mv $$lang.po $$lang.old.po; \
	  if $(MSGMERGE) $$lang.old.po $(srcdir)/src/nls/$(PACKAGE).pot > $$lang.po; then \
	    rm -f $$lang.old.po; \
	  else \
	    echo "msgmerge for $$cat failed!"; \
	    rm -f $$lang.po; \
	    mv $$lang.old.po $$lang.po; \
	    chmod 644 $$lang.po; \
	  fi; \
	done

distclean: clean depclean
	rm -f $(alltarg) src/include/config.h
	rm -rf $(package)-$(version).tar* $(package)-$(version) $(package)-$(version)-*.rpm
	rm -f *.html config.*
	rm Makefile

cvsclean svnclean: distclean
	rm -f doc/lsm
	rm -f doc/$(package).spec
	rm -f doc/quickref.1
	rm -f configure
	rm -f src/nls/*.gmo src/nls/*.mo
	echo > $(srcdir)/autoconf/make/depend.mk~
	echo > $(srcdir)/autoconf/make/filelist.mk~
	echo > $(srcdir)/autoconf/make/modules.mk~

doc:
	:

index:
	(cd $(srcdir); sh autoconf/scripts/index.sh $(srcdir)) > index.html

manhtml:
	@man2html ./doc/quickref.1 \
	| sed -e '1,/<BODY/d' -e '/<\/BODY/,$$d' \
	      -e 's|<A [^>]*>&nbsp;</A>||ig' \
	      -e 's|<A [^>]*>\([^<]*\)</A>|\1|ig' \
	      -e '/<H1/d' -e 's|\(</H[0-9]>\)|\1<P>|ig' \
	      -e 's/<DL COMPACT>/<DL>/ig' \
	      -e 's/&lt;[0-9A-Za-z_.-]\+@[0-9A-Za-z_.-]\+&gt;//g' \
	      -e 's|<I>\(http://.*\)</I>|<A HREF="\1">\1</A>|ig' \
	| sed -e '1,/<HR/d' -e '/<H2>Index/,/<HR/d' \

indent:
	cd $(srcdir) && indent -npro -kr -i8 -cd42 -c45 $(allsrc)

dist: doc update-po
	rm -rf $(package)-$(version)
	mkdir $(package)-$(version)
	cp -dprf Makefile $(distfiles) $(package)-$(version)
	cd $(package)-$(version); $(MAKE) distclean
	cp -dpf doc/lsm             $(package)-$(version)/doc/
	cp -dpf doc/$(package).spec $(package)-$(version)/doc/
	chmod 644 `find $(package)-$(version) -type f -print`
	chmod 755 `find $(package)-$(version) -type d -print`
	chmod 755 `find $(package)-$(version)/autoconf/scripts`
	chmod 755 $(package)-$(version)/configure
	rm -rf DUMMY `find $(package)-$(version) -type d -name CVS`
	rm -rf DUMMY `find $(package)-$(version) -type d -name .svn`
	tar cf $(package)-$(version).tar $(package)-$(version)
	rm -rf $(package)-$(version)
	-gzip -f9 $(package)-$(version).tar

check test: $(alltarg)
	sh $(srcdir)/autoconf/scripts/run-test.sh ./$(package) $(srcdir)

install: all doc
	$(srcdir)/autoconf/scripts/mkinstalldirs \
	  "$(DESTDIR)$(bindir)"
	$(srcdir)/autoconf/scripts/mkinstalldirs \
	  "$(DESTDIR)$(mandir)/man1"
	$(INSTALL) -m 755 $(package) \
	                  "$(DESTDIR)$(bindir)/$(package)"
	$(INSTALL) -m 644 doc/quickref.1 \
	                  "$(DESTDIR)$(mandir)/man1/$(package).1"
	if test -n "$(CATALOGS)"; then \
	  catalogs='$(CATALOGS)'; \
	  for cat in $$catalogs; do \
	    name=`echo $$cat | sed 's,^.*/,,g'`; \
	    if test "`echo $$name | sed 's/.*\(\..*\)/\1/'`" = ".gmo"; then \
	      destdir=$(gnulocaledir); \
	    else \
	      destdir=$(localedir); \
	    fi; \
	    lang=`echo $$name | sed 's/$(CATOBJEXT)$$//'`; \
	    dir=$(DESTDIR)$$destdir/$$lang/LC_MESSAGES; \
	    $(srcdir)/autoconf/scripts/mkinstalldirs $$dir; \
	    $(INSTALL_DATA) $$cat $$dir/$(PACKAGE)$(INSTOBJEXT); \
	  done; \
	fi

uninstall:
	-$(UNINSTALL) "$(DESTDIR)$(bindir)/$(package)"
	-$(UNINSTALL) "$(DESTDIR)$(mandir)/man1/$(package).1"
	-$(UNINSTALL) "$(DESTDIR)$(mandir)/man1/$(package).1.gz"
	-if test -n "$(CATALOGS)"; then \
	  catalogs='$(CATALOGS)'; \
	  for cat in $$catalogs; do \
	    name=`echo $$cat | sed 's,^.*/,,g'`; \
	    if test "`echo $$name | sed 's/.*\(\..*\)/\1/'`" = ".gmo"; then \
	      destdir=$(gnulocaledir); \
	    else \
	      destdir=$(localedir); \
	    fi; \
	    lang=`echo $$name | sed 's/$(CATOBJEXT)$$//'`; \
	    dir=$(DESTDIR)$$destdir/$$lang/LC_MESSAGES; \
	    $(UNINSTALL) $$dir/$(PACKAGE)$(INSTOBJEXT); \
	  done; \
	fi

rpm:
	test -e $(package)-$(version).tar.gz || $(MAKE) dist
	rpmbuild $(RPMFLAGS) --define="%_topdir `pwd`/rpm" -tb $(package)-$(version).tar.gz
	mv rpm/RPMS/*/$(package)-*.rpm .
	rm -rf rpm

srpm:
	test -e $(package)-$(version).tar.gz || $(MAKE) dist
	rpmbuild $(RPMFLAGS) --define="%_topdir `pwd`/rpm" -ts $(package)-$(version).tar.gz
	mv rpm/SRPMS/*$(package)-*.rpm .
	rm -rf rpm

release: dist rpm srpm
	zcat $(package)-$(version).tar.gz | bzip2 > $(package)-$(version).tar.bz2
	-grep -Fq '%_gpg_name' ~/.rpmmacros 2>/dev/null && rpm --addsign *.rpm
	-gpg --list-secret-keys 2>&1 | grep -Fq 'uid' && gpg -ab *.tar.gz && rename .asc .txt *.tar.gz.asc
	-gpg --list-secret-keys 2>&1 | grep -Fq 'uid' && gpg -ab *.tar.bz2 && rename .asc .txt *.tar.bz2.asc
	chmod 644 $(package)-$(version)*
# Automatically generated module linking rules
#
# Creation time: Sun Mar 15 00:38:16 GMT 2015

src/library.o:  src/library/getopt.o src/library/gettext.o
	$(LD) $(LDFLAGS) -o $@  src/library/getopt.o src/library/gettext.o

src/pv.o:  src/pv/cursor.o src/pv/display.o src/pv/file.o src/pv/loop.o src/pv/number.o src/pv/signal.o src/pv/state.o src/pv/transfer.o src/pv/watchpid.o
	$(LD) $(LDFLAGS) -o $@  src/pv/cursor.o src/pv/display.o src/pv/file.o src/pv/loop.o src/pv/number.o src/pv/signal.o src/pv/state.o src/pv/transfer.o src/pv/watchpid.o

src/main.o:  src/main/debug.o src/main/help.o src/main/main.o src/main/options.o src/main/remote.o src/main/version.o
	$(LD) $(LDFLAGS) -o $@  src/main/debug.o src/main/help.o src/main/main.o src/main/options.o src/main/remote.o src/main/version.o


#
# Compilation rules.
#

.SUFFIXES: .c .d .o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.c.d:
	sh $(srcdir)/autoconf/scripts/depend.sh \
	   $(CC) $< $(<:%.c=%) $(srcdir) $(CFLAGS) $(CPPFLAGS) > $@

#
# NLS stuff
#

%.mo: %.po
	$(MSGFMT) -o $@ $<
	@touch $@
	@chmod 644 $@

%.gmo: %.po
	rm -f $@
	$(GMSGFMT) -o $@ $<
	@touch $@
	@chmod 644 $@

$(srcdir)/src/nls/$(PACKAGE).pot: $(allsrc)
	$(XGETTEXT) --default-domain=$(PACKAGE) --directory=$(srcdir) \
	            --add-comments --keyword=_ --keyword=N_ \
	            $(allsrc)
	if cmp -s $(PACKAGE).po $@; then \
	  rm -f $(PACKAGE).po; \
	else \
	  rm -f $@; \
	  mv $(PACKAGE).po $@; \
	  chmod 644 $@; \
	fi

src/nls/table.c: $(POFILES)
	sh $(srcdir)/autoconf/scripts/po2table.sh $(POFILES) > src/nls/table.c

#
# Targets.
#

$(package): src/main.o src/library.o src/pv.o 
	$(CC) $(LINKFLAGS) $(CFLAGS) -o $@ src/main.o src/library.o src/pv.o  $(LIBS)

$(package)-static: src/main.o src/library.o src/pv.o 
	$(CC) $(LINKFLAGS) $(CFLAGS) -static -o $@ src/main.o src/library.o src/pv.o  $(LIBS)

# EOF
#
# Dependencies.
#

src/library/getopt.d src/library/getopt.o: src/library/getopt.c src/include/config.h src/include/library/gettext.h 
src/library/gettext.d src/library/gettext.o: src/library/gettext.c src/include/config.h src/include/library/gettext.h 
src/pv/signal.d src/pv/signal.o: src/pv/signal.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/watchpid.d src/pv/watchpid.o: src/pv/watchpid.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/cursor.d src/pv/cursor.o: src/pv/cursor.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/file.d src/pv/file.o: src/pv/file.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/display.d src/pv/display.o: src/pv/display.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/loop.d src/pv/loop.o: src/pv/loop.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/number.d src/pv/number.o: src/pv/number.c src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/transfer.d src/pv/transfer.o: src/pv/transfer.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/pv/state.d src/pv/state.o: src/pv/state.c src/include/pv-internal.h src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/main/version.d src/main/version.o: src/main/version.c src/include/config.h src/include/library/gettext.h 
src/main/debug.d src/main/debug.o: src/main/debug.c src/include/config.h src/include/library/gettext.h src/include/pv.h 
src/main/main.d src/main/main.o: src/main/main.c src/include/config.h src/include/library/gettext.h src/include/options.h src/include/pv.h 
src/main/options.d src/main/options.o: src/main/options.c src/include/config.h src/include/library/gettext.h src/include/options.h src/include/library/getopt.h src/include/pv.h 
src/main/remote.d src/main/remote.o: src/main/remote.c src/include/config.h src/include/library/gettext.h src/include/options.h src/include/pv.h 
src/main/help.d src/main/help.o: src/main/help.c src/include/config.h src/include/library/gettext.h 
