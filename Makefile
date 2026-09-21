# lispovckiy (c) 2026

PREFIX = /usr/local
INCS = -I/usr/include -I/usr/X11R6/include -Isrc/include
LIBS = -L/usr/lib -L/usr/X11R6/lib -lX11
CFLAGS = -Wall -Wextra -O2 ${INCS}
LDFLAGS = ${LIBS}
OBJ = src/lispm.o src/bsp.o
all: lispm lispmc
lispm: ${OBJ}
	$(CC) -o $@ ${OBJ} ${LDFLAGS}
lispmc: src/lispmc.c
	$(CC) $(CFLAGS) src/lispmc.c -o lispmc $(LDFLAGS)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -f lispm ${OBJ}
install: all
	cp -f lispmc ${DESTDIR}${PREFIX}/bin/lispmc
	chmod 755 ${DESTDIR}${PREFIX}/bin/lispmc
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f lispm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/lispm
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/lispm
.PHONY: all clean install uninstall

