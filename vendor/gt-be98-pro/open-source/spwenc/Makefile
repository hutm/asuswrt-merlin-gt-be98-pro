# Relocatable source replacement for the GT-BE98 Pro shared-library component.
.DEFAULT_GOAL := all
SOURCE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= $(CURDIR)/.open-spwenc
OUTPUT ?= $(CURDIR)/open-spwenc.o
SOURCES := access.c aes_abi.c commands.c crypto.c frame.c helpers.c init.c inputs.c key.c lengths.c md5_abi.c migration.c names.c password.c policy.c policy_init.c settings.c timestamps.c wrapper.c
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
HEADERS := $(wildcard $(SOURCE_DIR)*.h)
.PHONY: all clean
all: $(OUTPUT)
$(BUILD_DIR):
	mkdir -p $@
$(BUILD_DIR)/%.o: $(SOURCE_DIR)%.c $(HEADERS) $(SOURCE_DIR)Makefile | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -std=gnu11 -fPIC -c $< -o $@
$(OUTPUT): $(OBJECTS)
	$(CC) -nostdlib -no-pie -Wl,-r -o $@ $(OBJECTS)
clean:
	rm -f $(OBJECTS) $(OUTPUT)
