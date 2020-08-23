# Makefile for clamav-realtime v0.1.0
# Including platform definitions

NAME=clamav-realtime
VERSION=0.1.0

include Makefile.defs

CC=		gcc
STD=		_GNU_SOURCE
OBJS=		functions.o main.o
LIBS=		-lnotify -lclamav
PKGCONFIG=	`pkg-config --cflags --libs libnotify libclamav`

.c.o:
		@echo Compiling sources...
		$(CC) -c -Wall $(CFLAGS) -D$(STD) $(PKGCONFIG) $<

build: $(OBJS)
		@echo Building executable...
		$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(NAME)

all: build install clean
		@echo $(NAME) was successfully installed !

install:
		@echo Installing files...
		install -d $(DESTDIR)/$(PREFIX)
		@chmod 655 $(NAME)
		@cp $(NAME) $(DESTDIR)/$(PREFIX)
		install -d $(DESTDIR)/$(PIXMAPS)
		@cp files/$(NAME).png $(DESTDIR)/$(PIXMAPS)
		install -d $(DESTDIR)/$(APPS)
		@cp files/$(NAME).desktop $(DESTDIR)/$(APPS)
		install -d $(DESTDIR)/$(DOCDIR)/$(NAME)-$(VERSION)
		for doc in $(DOCFILES); do
			cp $$doc $(DESTDIR)/$(DOCDIR)/$(NAME)-$(VERSION);
		done

clean:
		@echo Cleaning sources...
		rm -f $(NAME) *.o core

clobber: clean
		@echo Uninstall application...
		rm -f $(PREFIX)/$(NAME)
		rm -f $(PIXMAPS)/$(NAME).png
		rm -f $(APPS)/$(NAME).desktop
		[ -e $(DOCDIR)/$(NAME)-$(VERSION) ] && rm -Rf $(DOCDIR)/$(NAME)-$(VERSION);
		@echo $(NAME) was successfully uninstalled !
