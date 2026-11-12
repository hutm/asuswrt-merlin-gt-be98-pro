SOURCE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= .open-amas-utils
OUTPUT ?= amas_utils.o
SOURCES := acsd-init backhaul backhaul-setting band-map band-type channel-info channel-weights configuration helpers interfaces mlo-config mlo-policy mlo-stage mlo-support parse-ethernet radio-policy settings uplink-description
OBJECTS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(SOURCES)))

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -r $^ -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)
