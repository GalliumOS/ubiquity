CFLAGS = -W -Wall

ifdef DEBUG
CFLAGS += -g3
endif

STRIP = strip

all: find-partitions mkfstab mountpoint

find-partitions: partconf.h find-parts.c util.o xasprintf.o
	$(CC) $(CFLAGS) -o $@ -DFIND_PARTS_MAIN find-parts.c util.o xasprintf.o -lparted

mkfstab: LDLIBS := -ldebian-installer
mkfstab: mkfstab.o xasprintf.o

mountpoint: LDLIBS := -ldebian-installer
mountpoint: mountpoint.o

xasprintf.o: xasprintf.c xasprintf.h
util.o: util.c
find-parts.o: find-parts.c partconf.h

small: CFLAGS += -Os
small: clean find-partitions mkfstab mountpoint
	$(STRIP) --remove-section=.comment --remove-section=.note find-partitions
	$(STRIP) --remove-section=.comment --remove-section=.note mkfstab
	$(STRIP) --remove-section=.comment --remove-section=.note mountpoint

test-mkfstab: mkfstab.c mkfstab.h
	$(CC) $(CFLAGS) -o test-mkfstab -DTEST mkfstab.c xasprintf.o -lparted -ldebian-installer

check: test-mkfstab
	./test-mkfstab

clean:
	-rm -f find-partitions mkfstab mountpoint \
		test-mkfstab *.o

.PHONY: all check clean small
