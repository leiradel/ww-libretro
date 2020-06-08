%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

%.lua.h : %.lua
	xxd -i $< \
		| sed "s/src_lua_//" \
		| sed "s/unsigned char/static const char/" \
		| sed "s/unsigned int/static const size_t/" \
		> $@

CC ?= gcc
CFLAGS = -std=c99 -Wall -Wpedantic -Werror -fPIC
INCLUDES = -Isrc -Isrc/libpng
LIBS = -lm

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -g -DWW_DEBUG
else
	CFLAGS += -O3 -DWW_RELEASE -DNDEBUG
endif

LIBPNG_OBJ_FILES = \
	src/libpng/png.o \
	src/libpng/pngerror.o \
	src/libpng/pngget.o \
	src/libpng/pngmem.o \
	src/libpng/pngread.o \
	src/libpng/pngrio.o \
	src/libpng/pngrtran.o \
	src/libpng/pngrutil.o \
	src/libpng/pngset.o \
	src/libpng/pngtrans.o \
	src/libpng/pngwio.o \
	src/libpng/pngwrite.o \
	src/libpng/pngwtran.o \
	src/libpng/pngwutil.o

ZLIB_OBJ_FILES = \
	src/zlib/adler32.o \
	src/zlib/crc32.o \
	src/zlib/deflate.o \
	src/zlib/inffast.o \
	src/zlib/inflate.o \
	src/zlib/inftrees.o \
	src/zlib/trees.o \
	src/zlib/zutil.o

WW_OBJ_FILES = \
	src/ww_backgrnd.o \
	src/ww_filesys.o \
	src/ww_screen.o \
	src/ww_sprite.o \
	src/ww_tile.o

OBJ_FILES = \
	$(WW_OBJ_FILES) \
	$(LIBPNG_OBJ_FILES) \
	$(ZLIB_OBJ_FILES)

all: src/ww_version.h ww_libretro.so

ww_libretro.so: src/libretro/libretro.o $(OBJ_FILES)
	$(CC) -shared -o $@ $+ $(LIBS)

src/ww_version.h: FORCE
	cat etc/ww_version.h.templ \
		| sed s/%HASH/`git rev-parse HEAD | tr -d "\n"`/g \
		| sed s/%VERSION/`git tag | sort -r -V | head -n1 | tr -d "\n"`/g \
		> $@

clean: FORCE
	rm -f ww_libretro.so src/libretro/libretro.o $(OBJ_FILES)
	rm -f src/ww_version.h

.PHONY: FORCE
