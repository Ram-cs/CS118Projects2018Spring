CC=gcc
CPPFLAGS=-g -Wall
USERID=604841556
CLASSES=

all: webserver

webserver: $(CLASSES)
	$(CC) -o $@ $^ $(CPPFLAGS) $@.c

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM webserver *.tar.gz

dist: tarball

tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
