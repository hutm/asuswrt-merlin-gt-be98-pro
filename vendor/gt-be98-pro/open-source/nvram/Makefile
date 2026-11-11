# Experimental GT-BE98 Pro runtime library; caller selects target compiler.
.DEFAULT_GOAL := all
SOURCE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OUTPUT ?= $(CURDIR)/libnvram.so
SHARED_DIR ?= $(error SHARED_DIR must point to the target libshared.so directory)
SOURCES := core.c adapter.c storage_binding.c storage_lock.c runtime.c compat/bitflags.c compat/strings.c compat/numbers.c
INPUTS := $(addprefix $(SOURCE_DIR),$(SOURCES))
HEADERS := $(wildcard $(SOURCE_DIR)*.h)
.PHONY: all FORCE
FORCE:
all: $(OUTPUT)
$(OUTPUT): FORCE $(INPUTS) $(HEADERS) $(SOURCE_DIR)exports.map $(SOURCE_DIR)Makefile
	$(CC) -std=c11 -Os -Wall -Wextra -Werror -fPIC -pthread -shared \
	  -Wl,-soname,libnvram.so -Wl,--version-script=$(SOURCE_DIR)exports.map \
	  -o $@ $(INPUTS) -L$(SHARED_DIR) -lshared
