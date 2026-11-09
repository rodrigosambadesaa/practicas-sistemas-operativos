# Compilacion de las practicas POSIX en Linux y macOS.
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
CPPFLAGS += -D_POSIX_C_SOURCE=200809L
ifeq ($(shell uname -s),Darwin)
CPPFLAGS += -D_DARWIN_C_SOURCE=1
else
CPPFLAGS += -D_DEFAULT_SOURCE=1
endif
LDLIBS += -lm -pthread

SOURCES := $(wildcard *.c)
PROGRAMS := $(patsubst %.c,build/%,$(SOURCES))

.PHONY: all test clean
all: $(PROGRAMS)

build:
	mkdir -p $@

build/%: %.c posix_compat.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

test: all
	python3 tests/smoke.py

clean:
	rm -rf build
