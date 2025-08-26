# Makefile for Cash Shell. Used claude to make this
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = 

SRCDIR = src
INCDIR = include
OBJDIR = obj

TARGET = cash

SOURCES = $(SRCDIR)/cash.c $(SRCDIR)/builtins.c
OBJECTS = $(OBJDIR)/cash.o $(OBJDIR)/builtins.o

HEADERS = $(INCDIR)/cash.h $(INCDIR)/builtins.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/cash.o: $(SRCDIR)/cash.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/cash.c -o $(OBJDIR)/cash.o

$(OBJDIR)/builtins.o: $(SRCDIR)/builtins.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/builtins.c -o $(OBJDIR)/builtins.o

clean:
	rm -rf $(OBJDIR) $(TARGET)

rebuild: clean all

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

info:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "TARGET: $(TARGET)"

.PHONY: all clean rebuild install uninstall debug release info