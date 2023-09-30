bin_PROGRAMS=psplash psplash-write

AM_CFLAGS = $(GCC_FLAGS) $(EXTRA_GCC_FLAGS) -D_GNU_SOURCE -DFONT_HEADER=\"$(FONT_NAME)-font.h\" -DFONT_DEF=$(FONT_NAME)_font

psplash_SOURCES = psplash.c psplash.h psplash-fb.c psplash-fb.h \
                  psplash-console.c psplash-console.h           \
		  psplash-colors.h psplash-config.h		\
		  psplash-neobox-img.h psplash-bar-img.h $(FONT_NAME)-font.h
BUILT_SOURCES = neobox-320x480-gray-img.h neobox-320x480-img.h psplash-bar-img.h

psplash_write_SOURCES = psplash-write.c psplash.h

if HAVE_SYSTEMD
psplash_CPPFLAGS = $(SYSTEMD_CFLAGS) -DHAVE_SYSTEMD
psplash_LDFLAGS= $(SYSTEMD_LIBS)
bin_PROGRAMS += psplash-systemd
psplash_systemd_CPPFLAGS = $(SYSTEMD_CFLAGS)
psplash_systemd_LDFLAGS= $(SYSTEMD_LIBS)
psplash_systemd_SOURCES = psplash-systemd.c psplash.h
endif

EXTRA_DIST = make-image-header.sh

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure depcomp install-sh ltmain.sh Makefile.in missing

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

psplash-bar-img.h: base-images/psplash-bar.png
	$(top_srcdir)/make-image-header.sh $< BAR
neobox-320x480-gray-img.h: base-images/neobox-320x480-gray.png
	$(top_srcdir)/make-image-header.sh $< NEOBOX_GRAY
neobox-320x480-img.h: base-images/neobox-320x480.png
	$(top_srcdir)/make-image-header.sh $< NEOBOX

