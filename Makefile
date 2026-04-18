ifeq ($(OS),Windows_NT)
PLATFORM_TAG := windows
else
PLATFORM_TAG := linux
endif

BUILD_DIR ?= build/$(PLATFORM_TAG)
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
LIB_DIR := $(BUILD_DIR)/lib
LIB_NAME := $(LIB_DIR)/libaxidev-io.a

ifeq ($(BUILD_DIR),build/$(PLATFORM_TAG))
CLEAN_ROOT := build
else
CLEAN_ROOT := $(BUILD_DIR)
endif

ifeq ($(origin CC), default)
ifeq ($(OS),Windows_NT)
CC := clang
else
CC := cc
endif
endif

ifeq ($(origin AR), default)
ifeq ($(OS),Windows_NT)
AR := llvm-ar
else
AR := ar
endif
endif

PKG_CONFIG ?= pkg-config

CPPFLAGS := -Iinclude -Isrc -Ivendor
CFLAGS ?= -std=c11 -Wall -Wextra -Wno-unused-parameter
LDFLAGS ?=
LDLIBS ?=

COMMON_SOURCES := \
	src/c_api.c \
	src/core/context.c \
	src/core/log.c \
	src/internal/utf.c \
	src/vendor/stb_ds_impl.c \
	src/keyboard/common/key_utils.c \
	src/keyboard/common/keymap.c

UNIT_TEST_SOURCES := tests/test_key_utils.c tests/test_c_api.c
INTEGRATION_TEST_SOURCES := tests/test_integration_sender.c tests/test_integration_listener.c

ifeq ($(OS),Windows_NT)
EXE_EXT := .exe
THREAD_SOURCE := src/internal/thread_win32.c
PLATFORM_SOURCES := \
	src/keyboard/common/windows_keymap.c \
	src/keyboard/sender/sender_windows.c \
	src/keyboard/listener/listener_windows.c
PLATFORM_LIBS := -luser32 -lkernel32
CPPFLAGS += -D_CRT_SECURE_NO_WARNINGS -DAXIDEV_IO_STATIC
define make-dir
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(1)' | Out-Null"
endef
define remove-path
	powershell -NoProfile -Command "if (Test-Path '$(1)') { Remove-Item -Recurse -Force '$(1)' }"
endef
define run-bin
	.\$(1)
endef
else
EXE_EXT :=
THREAD_SOURCE := src/internal/thread_pthread.c
PLATFORM_SOURCES := \
	src/keyboard/common/linux_layout.c \
	src/keyboard/common/linux_keysym.c \
	src/keyboard/sender/sender_uinput.c \
	src/keyboard/listener/listener_linux.c
PLATFORM_CFLAGS := $(shell $(PKG_CONFIG) --cflags libinput libudev xkbcommon 2>/dev/null)
PLATFORM_LIBS := $(shell $(PKG_CONFIG) --libs libinput libudev xkbcommon 2>/dev/null) -pthread
CPPFLAGS += -DAXIDEV_IO_STATIC $(PLATFORM_CFLAGS)
define make-dir
	mkdir -p '$(1)'
endef
define remove-path
	rm -rf '$(1)'
endef
define run-bin
	./$(1)
endef
endif

LIB_SOURCES := $(COMMON_SOURCES) $(THREAD_SOURCE) $(PLATFORM_SOURCES)
LIB_OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIB_SOURCES))
UNIT_TEST_BINS := $(patsubst tests/%.c,$(BIN_DIR)/%$(EXE_EXT),$(UNIT_TEST_SOURCES))
INTEGRATION_TEST_BINS := $(patsubst tests/%.c,$(BIN_DIR)/%$(EXE_EXT),$(INTEGRATION_TEST_SOURCES))
EXAMPLE_BIN := $(BIN_DIR)/example_c$(EXE_EXT)

.PHONY: all build test test-unit test-integration example clean

all: build

build: $(LIB_NAME)

test: test-unit

test-unit: $(UNIT_TEST_BINS)
	@$(call run-bin,$(BIN_DIR)/test_key_utils$(EXE_EXT))
	@$(call run-bin,$(BIN_DIR)/test_c_api$(EXE_EXT))

test-integration: $(INTEGRATION_TEST_BINS)
	@$(call run-bin,$(BIN_DIR)/test_integration_sender$(EXE_EXT))
	@$(call run-bin,$(BIN_DIR)/test_integration_listener$(EXE_EXT))

example: $(EXAMPLE_BIN)

$(LIB_NAME): $(LIB_OBJECTS)
	@$(call make-dir,$(@D))
	$(AR) rcs $@ $(LIB_OBJECTS)

$(OBJ_DIR)/%.o: %.c
	@$(call make-dir,$(@D))
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/test_key_utils$(EXE_EXT): tests/test_key_utils.c $(LIB_NAME)
	@$(call make-dir,$(@D))
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB_NAME) $(LDFLAGS) $(PLATFORM_LIBS) -o $@

$(BIN_DIR)/test_c_api$(EXE_EXT): tests/test_c_api.c $(LIB_NAME)
	@$(call make-dir,$(@D))
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB_NAME) $(LDFLAGS) $(PLATFORM_LIBS) -o $@

$(BIN_DIR)/test_integration_sender$(EXE_EXT): tests/test_integration_sender.c $(LIB_NAME)
	@$(call make-dir,$(@D))
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB_NAME) $(LDFLAGS) $(PLATFORM_LIBS) -o $@

$(BIN_DIR)/test_integration_listener$(EXE_EXT): tests/test_integration_listener.c $(LIB_NAME)
	@$(call make-dir,$(@D))
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB_NAME) $(LDFLAGS) $(PLATFORM_LIBS) -o $@

$(EXAMPLE_BIN): examples/example_c.c $(LIB_NAME)
	@$(call make-dir,$(@D))
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB_NAME) $(LDFLAGS) $(PLATFORM_LIBS) -o $@

clean:
	@$(call remove-path,$(CLEAN_ROOT))
