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

# Opt-in CLI, built against the same source runtime selected above.
CLI_OUTPUT ?= $(CURDIR)/nvram
CLI_SOURCES := cli_kernel.c cli_flags.c manufacturing.c profile.c
.PHONY: cli
cli: FORCE
	$(CC) -std=gnu99 -Os -DASUS_OPEN_NVRAM_CLI -DHND_ROUTER \
	  -I$(SOURCE_DIR) -I$(ROUTER_DIR)/shared \
	  -I$(PLATFORM_DIR)/bcmdrivers/broadcom/net/wl/impl103/main/src/include \
	  $(ROUTER_DIR)/nvram/nvram.c $(addprefix $(SOURCE_DIR),$(CLI_SOURCES)) \
	  -L$(NVRAM_DIR) -L$(SHARED_DIR) -L$(INSTALL_DIR)/lib -L$(INSTALL_DIR)/usr/lib \
	  -Wl,-rpath-link,$(ROUTER_DIR)/openssl-3.5:$(ROUTER_DIR)/zlib:$(INSTALL_DIR)/lib:$(INSTALL_DIR)/usr/lib \
	  -lnvram -lshared -o $(CLI_OUTPUT)
