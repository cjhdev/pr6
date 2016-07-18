DIR_ROOT := ../..

DIR_MODULE_DEBUG_INTERFACE := $(DIR_ROOT)/modules/debug_interface
DIR_MODULE_CORE := $(DIR_ROOT)/modules/core
DIR_MODULE_RCORE := $(DIR_ROOT)/modules/rcore

DEBUG_EXTERN_FUNCTION := void EXT_DebugPutErr(const char *function, const char *fmt, ...);
DEBUG := EXT_DebugPutErr(__FUNCTION__, __VA_ARGS__); 

DEFINES := -D'DEBUG_EXTERN_FUNCTION=$(DEBUG_EXTERN_FUNCTION)' -D'DEBUG(...)=$(DEBUG)'

INCLUDES := -I$(DIR_MODULE_CORE)/include -I$(DIR_MODULE_DEBUG_INTERFACE)/include
INCLUDES += -I$(RUBY_HDR_DIR) -I$(RUBY_ARCH_DIR)

CC := gcc
CFLAGS := -fPIC -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wall -fno-strict-aliasing $(DEFINES) $(INCLUDES)
LDFLAGS := -Wl,-Bsymbolic-functions -Wl,-z,relro -rdynamic -Wl,-export-dynamic -shared

VPATH := $(DIR_MODULE_CORE)/src:$(DIR_MODULE_RCORE)/ext/ext_wrangle

DIR_BUILD := build
DIR_OUTPUT := lib/wrangle

all: ext_wrangle

ext_wrangle: $(addprefix $(DIR_BUILD)/, pr6_server.o pr6_client.o pr6_encoder_decoder.o pr6_peer.o ext_pr6_server.o ext_pr6_client.o ext_pr6_peer.o ext_common.o ext_debug.o)
	$(CC) $(LDFLAGS) $(^) -o $(DIR_OUTPUT)/$(@).so -lfl

$(DIR_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) $(DIR_BUILD)/*
