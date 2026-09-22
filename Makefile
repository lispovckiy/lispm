# (c) 2026 lispovckiy
PREFIX ?= /usr/local
INCS    = -I/usr/include -I/usr/X11R6/include -Isrc/include
LIBS    = -L/usr/lib -L/usr/X11R6/lib -lX11
CFLAGS  = -Wall -Wextra -O3 -march=native -fomit-frame-pointer ${INCS}
LDFLAGS = ${LIBS}
OBJ     = src/lispm.o src/bsp.o src/handle_client_message.o
BIN     = lispm lispmc
all: ${BIN}
lispm: ${OBJ}
	$(CC) -o $@ ${OBJ} ${LDFLAGS}
lispmc: src/lispmc.c
	$(CC) $(CFLAGS) $< -o $@ ${LDFLAGS}
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -f ${BIN} ${OBJ}
install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f lispm ${DESTDIR}${PREFIX}/bin/lispm
	chmod 755 ${DESTDIR}${PREFIX}/bin/lispm
	cp -f lispmc ${DESTDIR}${PREFIX}/bin/lispmc
	chmod 755 ${DESTDIR}${PREFIX}/bin/lispmc
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/lispm
	rm -f ${DESTDIR}${PREFIX}/bin/lispmc
.PHONY: all clean install uninstall
