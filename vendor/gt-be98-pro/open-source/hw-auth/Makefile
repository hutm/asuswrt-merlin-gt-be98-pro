SOURCE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR ?= .open-hw-auth
OUTPUT ?= hw_auth.o
SOURCES := eligibility component check clm handshake tables validate
OBJECTS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(SOURCES)))

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -r $^ -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)
