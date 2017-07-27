
# ----------------------------------------------------------- #

.PHONY: all depend clean realclean mostlyclean dist

# -DDEBUG_MEMORY_USAGE : In der Zusammenfassung wird aufgefuehrt, wiviele bytes ungefaehr gebraucht wurden.

CC	= gcc
CFLAGS  = -g -Wall -W 
LDFLAGS = -lm
LINT    = lclint

VERSION:=$(shell grep "^\#define RELEASE_VERSION" atlantis.h | awk '{print $$3}')

# ----------------------------------------------------------- #

all: atlantis

# ----------------------------------------------------------- #

# Werden die #include's geaendert, sollte "make depend" erneut
# ausgefuehrt werden.

depend:
	$(CC) -MM *.c >makefile.dep

include makefile.dep

# ----------------------------------------------------------- #

# Fuer Entwickler bietet sich die find-tag Funktion von Emacs (und
# vim?) an.  So wird der TAGS file gemacht.

TAGS: *.c
	etags *.c

# ----------------------------------------------------------- #

language.c language.h: language.def mklang.pl
	./mklang.pl

# ----------------------------------------------------------- #

obj =	\
	atlantis.o \
	build.o \
	combat.o \
	constant.o \
	contact.o \
	creation.o \
	economic.o \
	give.o \
	indicato.o \
	language.o \
	laws.o \
	magic.o \
	main.o \
	monster.o \
	movement.o \
	reports.o \
	save.o \
	study.o \
	terrain.o \
	translate.o

src = $(obj:.o=.c)

atlantis: $(obj) 
	$(CC) $(LDFLAGS) -oatlantis $(obj)

lint: $(src)
	$(LINT) $^

# ----------------------------------------------------------- #

# Versucht alle unnoetigen files zu loeschen. Das file 'makefile.dep'
# kann mit "make depend" neu erzeugt werden. Wird es geloescht, wird
# make ueberhaupt nicht funktionieren, deswegen nie ganz loeschen.

# Die Daten im /DATA file sind aeusserst wichtig (vor allem dasjenige
# mit der hoechsten Nummer!). Diese muessen per Hand geloescht werden.
# Die Reports in /REPORTS koennen mit "atlantis -r" neu erzeugt werden,
# vorausgesetzt der letzte /DATA file existiert. /REPORTS werden mit
# "make realclean" geloescht.

mostlyclean:
	rm -f *.lst *.r *.rc

clean: mostlyclean
	rm -f *.o *.nr *.cr info* adressen* karte* parteien* *~ atlantis.tar.gz
	rm -rf atlantis-$(VERSION)

realclean: clean
	rm -f atlantis reports/*

# ----------------------------------------------------------- #

dist: atlantis-$(VERSION).tar.gz

ChangeLog: *.c *.h *.inc *.def *.pl Makefile makefile.dep *.txt
	cvs log > $@

atlantis-$(VERSION).tar.gz: *.c *.h *.inc *.def *.pl Makefile makefile.dep *.txt ChangeLog
	if test ! -d atlantis-$(VERSION); then mkdir atlantis-$(VERSION); fi
	ln -f $^ atlantis-$(VERSION)
	tar -czf $@ atlantis-$(VERSION)/*
	ln -f $@ atlantis.tar.gz
