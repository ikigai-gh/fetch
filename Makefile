SRC = fetch.c lib.c
VERSION = $(shell git tag --sort=taggerdate | tail -1)
X11 =
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib
LIBS =

CC = cc
# Optimized, without debug info, stripped
CFLAGS = -std=c99 -O3 -s -Wall -Wextra -Wpedantic $(INCS)
# Not optimized, with debug info, not stripped
CFLAGS_DEBUG = -std=c99 -O0 -g -Wall -Wextra -Wpedantic $(INCS)
CPPFLAGS = -DVERSION=\"${VERSION}\"
CPPFLAGS_DEBUG = -DDEBUG -DVERSION=\"${VERSION}\"
LDFLAGS = $(LIBS)

ifdef X11
	LIBS += -L${X11LIB} -lX11
	CPPFLAGS+= -DX11
endif

fetch: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) $(SRC) -o fetch

debug: $(SRC)
	$(CC) $(CFLAGS_DEBUG) $(LDFLAGS) $(CPPFLAGS_DEBUG) $(SRC) -o fetch
