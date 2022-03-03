PROGNAME = cfetch

CC = gcc
AS = as
LD = ld

INCS =
LIBS =

LDS = linker.ld
CFLAGS = $(LIBS) $(INCS)
ASFLAGS =
LDFLAGS = -T $(LDS)

SRCDIR := src
OBJDIR := lib
BUILDDIR := bin

PREFIX = /usr/local

RUNFLAGS =

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRC = $(call rwildcard,$(SRCDIR),*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
DIRS = $(wildcard $(SRCDIR)/*)

build: setup
	$(CC) $(CFLAGS) $(SRC) -o $(BUILDDIR)/$(PROGNAME)

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

run: build
	./$(BUILDDIR)/$(PROGNAME) $(RUNFLAGS)