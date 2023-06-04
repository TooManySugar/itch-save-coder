# VERBOSE == 1 -> echoing commands to console
ifneq ($(VERBOSE), 1)
VERB := @
endif

# Detect the OS
ifdef SystemRoot
OS_TYPE ?= Windows
else
OS_TYPE ?= $(shell uname -s)
endif

ifneq (,$(findstring MINGW,$(OS_TYPE)))
OS_TYPE := Windows
endif

# Target
TARGET := itch-save-coder

VERSION := 0.1

ifeq ($(OS_TYPE),Windows)
TARGET := $(addsuffix .exe,$(TARGET))
endif

CFLAGS ?= -O2 -Wall -std=c99

DEFINES := -DPROGRAM_NAME="\"$(TARGET)\"" -DVERSION="\"$(VERSION)\""

default: build

$(TARGET): main.c
	$(VERB) $(CC) $(CFLAGS) $(DEFINES) -o $@ $<

.PHONY: build
build: $(TARGET)