SOURCE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= .open-platform-private
OUTPUT ?= private.o
SOURCES := board-id channel-list channel-select channel-settings image-crc image-diagnostics link-status mdio-read package-board phy-bits phy-operation phy-status port-status rtk-counters rtk-properties switch-dispatch switch-registers sysfs trx-check vendor-ie vendor-ie-remove wan-status
OBJECTS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(SOURCES)))

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -r $^ -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)
