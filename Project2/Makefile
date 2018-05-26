CC=gcc
CPPFLAGS=-g -Wall
USERID=204650577
CLASSES=

all: client server
client: $(CLASSES)
	$(CC) -o $@ $^ $(CPPFLAGS) $@.c

server: $(CLASSES)
	$(CC) -o $@ $^ $(CPPFLAGS) $@.c

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM client server *.tar.gz

dist: tarball

tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
