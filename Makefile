PROGNAME = cfetch

CC = gcc
AS = as
LD = gcc

INCS =
LIBS =

LDS = linker.ld
CFLAGS = $(LIBS) $(INCS)
ASFLAGS =
LDFLAGS =

SRCDIR := src
OBJDIR := lib
BUILDDIR := bin

PREFIX = /usr/local

DB = gdb
VG = valgrind

VALFLAGS =
DBFLAGS =

RUNFLAGS =

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
DIRS = $(wildcard $(SRCDIR)/*)

build: setup link

link: $(OBJS)
	@ echo !==== LINKING $^
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/$(PROGNAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $^ -o $@ -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	$(AS) $(ASLAGS) $^ -f elf64 -o $@

setup:
	@ mkdir -p $(SRCDIR)
	@ mkdir -p $(OBJDIR)
	@ mkdir -p $(BUILDDIR)
	
clean:
	-@ rm -r $(BUILDDIR)
	-@ rm -r $(OBJDIR)

install: build
	@ cp $(BUILDDIR)/$(PROGNAME) $(PREFIX)/bin/$(PROGNAME)
	@ chmod 6555 $(PREFIX)/bin/$(PROGNAME)

run:
	./$(BUILDDIR)/$(PROGNAME) $(RUNFLAGS)

buildDebug:
	$(CC) $(CFLAGS) -g $(SRC) -o $(BUILDDIR)/$(PROGNAME)

debug: buildDebug
	$(DB) $(DEGUBERFLAGS) ./$(BUILDDIR)/$(PROGNAME) $(RUNFLAGS)

valgrind: buildDebug
	$(VG) $(VALFLAGS) ./$(BUILDDIR)/$(PROGNAME) $(RUNFLAGS)