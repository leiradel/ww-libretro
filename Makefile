%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

%.lua.h : %.lua
	xxd -i $< \
		| sed "s/src_lua_//" \
		| sed "s/unsigned char/static const char/" \
		| sed "s/unsigned int/static const size_t/" \
		> $@

CC ?= gcc
CFLAGS = -std=c99 -Wall -Wpedantic -DNOBYFOUR -fPIC
INCLUDES = -Isrc/zlib/contrib/minizip
LIBS = -llua -lm

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g -DWW_DEBUG
else
	CFLAGS += -O3 -DWW_RELEASE -DNDEBUG
endif

ZLIB_OBJ_FILES = \
	src/zlib/contrib/minizip/unzip.o \
	src/zlib/contrib/minizip/ioapi.o \
	src/zlib/crc32.o \
	src/zlib/adler32.o \
	src/zlib/inflate.o \
	src/zlib/inftrees.o \
	src/zlib/inffast.o \
	src/zlib/zutil.o

WW_OBJ_FILES = \
	src/ww_backgrnd.o \
	src/ww_image.o

OBJ_FILES = \
	$(WW_OBJ_FILES) \
	$(ZLIB_OBJ_FILES)

all: ww_libretro.so

ww_libretro.so: $(OBJ_FILES)
	$(CC) -shared -o $@ $+ $(LIBS)

src/ww_backgrnd.c: src/ww_version.h

src/ww_version.h: FORCE
	cat etc/ww_version.h.templ \
		| sed s/%HASH/`git rev-parse HEAD | tr -d "\n"`/g \
		| sed s/%VERSION/`git tag | sort -r -V | head -n1 | tr -d "\n"`/g \
		> $@

clean: FORCE
	rm -f ww_libretro.so $(OBJ_FILES)
	rm -f src/ww_version.h

.PHONY: FORCE
