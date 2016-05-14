DIR_ROOT := ../..

CC := gcc

DIR_UNITY := $(DIR_ROOT)/vendor/unity

DIR_MODULE_CORE := $(DIR_ROOT)/modules/core
DIR_MODULE_DEBUG_INTERFACE := $(DIR_ROOT)/modules/debug_interface

DIR_BUILD := build
DIR_BIN := bin
DIR_RUNNERS := runners

DEBUG_EXTERN_FUNCTION := void HostDebugPutErr(const char *function, const char *fmt, ...);
DEBUG := HostDebugPutErr(__FUNCTION__, __VA_ARGS__); 

VPATH := $(DIR_MODULE_CORE)/src:$(DIR_MODULE_CORE)/test/unit_test:$(DIR_UNITY)/src:$(DIR_RUNNERS)

DEFINES := -D'DEBUG_EXTERN_FUNCTION=$(DEBUG_EXTERN_FUNCTION)' -D'DEBUG(...)=$(DEBUG)'
INCLUDES := -I$(DIR_MODULE_CORE)/include -I$(DIR_MODULE_DEBUG_INTERFACE)/include -I$(DIR_UNITY)/src

CFLAGS := -Wall -std=c99 -g -fprofile-arcs -ftest-coverage $(DEFINES) $(INCLUDES)
LDFLAGS := -fprofile-arcs

TEST_FILES := $(shell ls $(DIR_MODULE_CORE)/test/unit_test)

TESTS := $(TEST_FILES:.c=)
TEST_RUNNERS := $(addprefix run_, $(TESTS))

TARGET_OBJ := debug_message.o
VPATH += target/host

all: run
#	echo $(TEST_RUNNERS)
#	echo $(VPATH)

runners: $(addprefix $(DIR_RUNNERS)/, $(TEST_RUNNERS:=.c))

$(DIR_RUNNERS)/run_%.c: %.c
	ruby $(DIR_UNITY)/auto/generate_test_runner.rb $(^) $(@)

tests: $(TEST_FILES:.c=)

#
# below we pattern match similar groups of test binaries
# 

tc_PR6_Get%: $(addprefix $(DIR_BUILD)/, run_tc_PR6_Get%.o tc_PR6_Get%.o pr6_encoder_decoder.o unity.o  $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@

tc_PR6_Put%: $(addprefix $(DIR_BUILD)/, run_tc_PR6_Put%.o tc_PR6_Put%.o pr6_encoder_decoder.o unity.o  $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@

tc_PR6_Peer%: $(addprefix $(DIR_BUILD)/, run_tc_PR6_Peer%.o tc_PR6_Peer%.o pr6_peer.o pr6_encoder_decoder.o unity.o  $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@
	
tc_PR6_SizeofVint: $(addprefix $(DIR_BUILD)/, run_tc_PR6_SizeofVint.o tc_PR6_SizeofVint.o pr6_encoder_decoder.o unity.o  $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@
	
tc_PR6_Cast%: $(addprefix $(DIR_BUILD)/, run_tc_PR6_Cast%.o tc_PR6_Cast%.o pr6_encoder_decoder.o unity.o  $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@

tc_PR6_Server%: $(addprefix $(DIR_BUILD)/, tc_PR6_Server%.o pr6_server.o pr6_encoder_decoder.o unity.o run_tc_PR6_Server%.o $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@

tc_PR6_Client%: $(addprefix $(DIR_BUILD)/, tc_PR6_Client%.o pr6_client.o pr6_encoder_decoder.o unity.o run_tc_PR6_Client%.o $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@

$(DIR_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean clean_obj clean_runners coverage

clean: clean_obj clean_runners

clean_obj:
	rm -f $(DIR_BUILD)/*

clean_runners:
	rm -f $(DIR_RUNNERS)/*

run: $(addprefix run_, $(TESTS))

run_%: %
	@-./$(DIR_BIN)/$^

coverage: $(addprefix $(DIR_BUILD)/, pr6_server.gcda pr6_encoder_decoder.gcda pr6_client.gcda pr6_peer.gcda)
	@echo "coverage report:"
	gcov --no-output --object-directory $(DIR_BUILD) $(^)



